%skeleton "lalr1.cc" 
%defines
%define parse.error verbose
%define api.value.type variant
%define api.token.constructor

%code requires {
	#include "Node.hpp"
		
	#define SHOW_DEBUG false
}

%code{
	#include <string>

	#define NEW_CHILD(parent, child) parent.children.push_back(child)
	#define YY_DECL yy::parser::symbol_type yylex()

	int processIndex = 0;

	#define COUNT "[" << processIndex++  << "]"
	#define MSG(msg) "Parsedline: " << __LINE__ << " " << COUNT << " " << msg
	#define PRINT_DEBUG(msg) if (SHOW_DEBUG) std::cout << MSG(msg) << std::endl;

	Node root;

	YY_DECL;
}

%left "==" "<" ">"
%right "%"

%left "+" "-" 
%left "*" "/"
%right "^"
%left "#"


%token <Node> NAME NUMBER STRING

%token PLUS "+"
%token SUB "-"
%token MUL "*"
%token DIV "/"
%token POW "^"

%token MOD "%"

%token COMP "=="
%token GREATER ">"
%token LESS "<"

%token OPENPAR "(" CLOSEPAR ")"
%token OPENSQUARE "[" CLOSESQUARE "]"
%token OPENCURLY "{" CLOSECURLY "}"

%token <Node> EQUAL "="

%token <Node> TRUE FALSE

%token SEMI ";"
%token COMA ","
%token DOT "."

%token <Node> LENGTH "#"

%token <Node>  FOR "for" IF "if" REPEAT "repeat" ELSE "else"
%token <Node>  FUNCTION "function" RETURN "return"
%token DO "do" END "end" THEN "then" UNTIL "until" 

%type <Node> chunk stat laststat args block
%type <Node> exp prefixexp explist var varlist
%type <Node> functioncall function funcbody parlist namelist
%type <Node> field fieldlist fieldsep tableconstructor unop

%token END_OF_FILE 0 "end of file"
%%
chunk:		stat					{	
										$$ = Node("chunk", "");
										NEW_CHILD($$, $1);
										root = $$;
										PRINT_DEBUG("chunk <- stat")
									}
			|laststat				{
										$$ = Node("chunk", "");
										NEW_CHILD($$, $1);
										root = $$;
										PRINT_DEBUG("chunk <- laststat")
									}
			|chunk ";"				{
										$$ = $1;	
										PRINT_DEBUG("") 
									}
			|chunk stat 			{	
										$$ = $1;
										NEW_CHILD($$, $2);
										root = $$;
										PRINT_DEBUG("")
									}
			|chunk laststat			{	
										$$ = $1;
										NEW_CHILD($$, $2);
										root = $$;
										PRINT_DEBUG("")
									}
			;

block:		chunk					{ 
										$1.tag = "block";
										$$ = $1;
										PRINT_DEBUG("")
									}
			;

stat:		varlist "=" explist					{ 
													$$ = $2;
													NEW_CHILD($$, $1);
													NEW_CHILD($$, $3);
													PRINT_DEBUG("stat <- " << $1.value << "=" << $3.children.front().value)
												}
			| functioncall						{
													$$ = $1;
													PRINT_DEBUG("stat <- f_call " << $1.value)
												}
			| "do" block "end"					{
													$$ = $2;
													PRINT_DEBUG("do block end <- " << $2.tag << " " << $2.value)
												}
			| "repeat" block "until" exp		{
													$$ = $1;
													NEW_CHILD($$, $2);
													NEW_CHILD($$, $4);
													PRINT_DEBUG("repeat <- " << $2.tag << " " << $2.value)
												}
			| "if" exp "then" block "end"		{
													$$ = $1;
													
													NEW_CHILD($$, $2); // iterator var
													NEW_CHILD($$, $4);
													
													PRINT_DEBUG("if <- ");
												}
			| "if" exp "then" block "else" block "end" 	{
															$$ = $1;
															
															NEW_CHILD($$, $2); // iterator var
															NEW_CHILD($$, $4);

															NEW_CHILD($5, $6);
															NEW_CHILD($$, $5);
															
															PRINT_DEBUG("if else <- ");
														}
			| "for" NAME "=" exp "," exp "do" block "end"			{
																		$$ = $1;

																		NEW_CHILD($$, $2); // iterator var
																		NEW_CHILD($$, $4);
																		
																		NEW_CHILD($$, $6); // loop to 
																		NEW_CHILD($$, Node("NUMBER", "1")); // step size

																		NEW_CHILD($$, $8); // block before because copy constructor

																		PRINT_DEBUG("for <- ");
																	}
			| "for" NAME "=" exp "," exp "," exp "do" block "end"	{
																		$$ = $1;
																		
																		NEW_CHILD($$, $2); // iterator var
																		NEW_CHILD($$, $4);
																		
																		NEW_CHILD($$, $6); // loop to 
																		NEW_CHILD($$, $8); // loop step size 

																		NEW_CHILD($$, $10); // block before because copy constructor

																		PRINT_DEBUG("for <- ");
																	}
			| "function" NAME funcbody			{
													$$ = $1;
													NEW_CHILD($$, $2);
													NEW_CHILD($$, $3);
													PRINT_DEBUG("function <- ");
												}
			;

laststat:	"return"				{
										$$ = $1;
										PRINT_DEBUG("");
									}
			| "return" explist		{
										$$ = $1;
										NEW_CHILD($$, $2);
										PRINT_DEBUG("");
									}
			;

varlist:	var						{
										$$ = Node("varlist", "");
										NEW_CHILD($$, $1);
										PRINT_DEBUG("");
									}
			| varlist "," var		{
										$$ = $1;
										NEW_CHILD($$, $3);
										PRINT_DEBUG("");
									}
			;

var:		NAME					{
										$$ = Node("var", $1.value);
										PRINT_DEBUG("var <- " <<  $1.tag << " " << $1.value)
									}
			| prefixexp "[" exp "]"	{
										$$ = Node("indexing", "");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);
										PRINT_DEBUG("var <- " << $1.value << "." << $3.value);
									}
			| prefixexp "." NAME	{
										$$ = Node("access", "");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);
										PRINT_DEBUG("var <- " << $1.value << "." << $3.value)
									}
			;

namelist:	NAME					{ 
										$$ = Node("namelist", "");
										NEW_CHILD($$, $1);
										PRINT_DEBUG("")
									}
			|NAME "," namelist		{ 
										$$ = $3;
										NEW_CHILD($$, $1);
										PRINT_DEBUG("")
									}

explist:	exp						{ 
										$$ = Node("explist", "");
										NEW_CHILD($$, $1);
										PRINT_DEBUG("")
									}
			|explist "," exp		{ 
										$$ = $1;
										NEW_CHILD($$, $3);
										PRINT_DEBUG("")
									}

exp:		NUMBER					{ $$ = $1; PRINT_DEBUG("exp <- NUMBER"); }
			|STRING					{ $$ = $1; PRINT_DEBUG("exp <- STRING"); }
			|TRUE					{ $$ = $1; PRINT_DEBUG("exp <- TRUE"); }
			|FALSE					{ $$ = $1; PRINT_DEBUG("exp <- FALSE"); }
			|prefixexp				{ $$ = $1; PRINT_DEBUG("exp <- prefixexp"); }
			|tableconstructor		{ $$ = $1; PRINT_DEBUG("exp <- tblconstructor"); }
			|function				{ $$ = $1; PRINT_DEBUG("exp <- function"); }
			|unop					{ 
										$$ = $1;
										PRINT_DEBUG("exp <- unop"); 
									}
			|exp "+" exp			{
										$$ = Node("binop", "+");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " + " << $3.value)
									}
			|exp "-" exp			{
										$$ = Node("binop", "-");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " - " << $3.value)
									}
			|exp "*" exp			{
										$$ = Node("binop", "*");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " * " << $3.value)
									}
			|exp "/" exp			{
										$$ = Node("binop", "/"); 
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " / " << $3.value)
									}
			|exp "^" exp			{
										$$ = Node("binop", "^");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " ^" << $3.value)
									}
			|exp "%" exp			{
										$$ = Node("binop", "%");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << " ^" << $3.value)
									}	
			|exp "==" exp			{
										$$ = Node("binop", "==");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << "==" << $3.value)
									}	
			|exp "<" exp			{
										$$ = Node("binop", "<");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << "==" << $3.value)
									}	
			|exp ">" exp			{
										$$ = Node("binop", ">");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);

										PRINT_DEBUG($1.value << "==" << $3.value)
									}	
			;

prefixexp:	var						{ $$ = $1; PRINT_DEBUG("prefixexp <- " << $1.tag << " " << $1.value); }
			|functioncall			{ $$ = $1; PRINT_DEBUG("prefixexp <- " << $1.tag << " " << $1.value); }
			|"(" exp ")"			{ $$ = $2; PRINT_DEBUG("prefixexp <- " << $2.tag << " " << $2.value); }
			;

functioncall: prefixexp args		{
										$$ = Node("f_call", "");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $2);

										PRINT_DEBUG("f_call: " << $1.value << " " << $2.value)
									}
			;

args:		"(" ")"					{ 
										$$ = Node("args", "empty"); 
										PRINT_DEBUG("")
									}
			|"(" explist ")"		{
										$2.tag = "args";
										$$ = $2;
										PRINT_DEBUG("")
									}
			|STRING					{
										
										$$ = Node("args", "");
										NEW_CHILD($$, $1);
										PRINT_DEBUG("")
									}
			;

function:	"function" funcbody		{
										$$ = $1;
										NEW_CHILD($$, $1);
										PRINT_DEBUG("")									
									}
			;

funcbody: "(" ")" block "end"				{
												$$ = Node("funcbody", "");
												NEW_CHILD($$, Node("namelist", ""));
												NEW_CHILD($$, $3);
												PRINT_DEBUG("")
											}
			| "(" parlist ")" block "end"	{
												$$ = Node("funcbody", "");
												NEW_CHILD($$, $2);
												NEW_CHILD($$, $4);
												PRINT_DEBUG("")
											}
			;

parlist:	namelist				{ $$ = $1; }
			;

tableconstructor: "{" "}"					{
												$$ = Node("tableconstructor", "");
												PRINT_DEBUG("")
											}
					| "{" fieldlist "}"		{
												$$ = $2;
												PRINT_DEBUG("")
											}
					;

fieldlist:	field							{
												$$ = Node("fieldlist", "");
												NEW_CHILD($$, $1);
												PRINT_DEBUG("")
											}
			| fieldlist fieldsep field		{
												$$ = $1;
												NEW_CHILD($$, $3);
												PRINT_DEBUG("")
											}
			;

field: 		"[" exp "]" "=" exp		{
										$$ = Node("field", "");
										NEW_CHILD($$, $2);
										NEW_CHILD($$, $5);
										PRINT_DEBUG("")
									}
			| NAME "=" exp			{
										$$ = Node("field", "");
										NEW_CHILD($$, $1);
										NEW_CHILD($$, $3);
										PRINT_DEBUG("")
									}
			| exp					{
										$$ = $1;
										PRINT_DEBUG("")
									}
			;

fieldsep: 	","						{
										PRINT_DEBUG("")
									}
			| ";"					{
										PRINT_DEBUG("")
									}
			;

unop:	"#"	exp						{
										$$ = $1;
										NEW_CHILD($$, $2);
										PRINT_DEBUG("unop")
									}
		;