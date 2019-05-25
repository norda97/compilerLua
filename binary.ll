%top{

#include "binary.tab.hh"
#include "Node.hpp"
#include <string>
#define YY_DECL yy::parser::symbol_type yylex()
}
%option noyywrap nounput batch noinput

%%
for								{ return yy::parser::make_FOR(Node("FOR", "")); }
if								{ return yy::parser::make_IF(Node("IF", "")); }
else							{ return yy::parser::make_ELSE(Node("ELSE", "")); }
repeat							{ return yy::parser::make_REPEAT(Node("REPEAT", "")); }
until							{ return yy::parser::make_UNTIL(); }
do								{ return yy::parser::make_DO(); }
end								{ return yy::parser::make_END(); }
then							{ return yy::parser::make_THEN(); }

function						{ return yy::parser::make_FUNCTION(Node("FUNCTION", "")); }
return							{ return yy::parser::make_RETURN(Node("RETURN", "")); }

true 							{ return yy::parser::make_TRUE(Node("TRUE", "")); }
false							{ return yy::parser::make_FALSE(Node("FALSE", "")); }

\"[^\"\n]*\"					{ std::string str(++yytext);str.pop_back(); return yy::parser::make_STRING(Node("STRING", str)); }

[a-zA-Z_]+[0-9a-zA-Z_]*			{ return yy::parser::make_NAME(Node("NAME", yytext)); }

[0-9]*\.?[0-9]+							{ return yy::parser::make_NUMBER(Node("NUMBER", yytext)); }

\%								{ return yy::parser::make_MOD(); }

\+								{ return yy::parser::make_PLUS(); }
\-								{ return yy::parser::make_SUB(); }
\*								{ return yy::parser::make_MUL(); }
\^								{ return yy::parser::make_POW(); }
\/								{ return yy::parser::make_DIV(); }

\=\=							{ return yy::parser::make_COMP(); }
\>								{ return yy::parser::make_GREATER(); }
\<								{ return yy::parser::make_LESS(); }

\=								{ return yy::parser::make_EQUAL(Node("EQUAL", yytext)); }

\(								{ return yy::parser::make_OPENPAR(); }
\)								{ return yy::parser::make_CLOSEPAR(); }
\{								{ return yy::parser::make_OPENCURLY(); }
\}								{ return yy::parser::make_CLOSECURLY(); }
\[								{ return yy::parser::make_OPENSQUARE(); }
\]								{ return yy::parser::make_CLOSESQUARE(); }

\;								{ return yy::parser::make_SEMI(); }
\,								{ return yy::parser::make_COMA(); }
\.								{ return yy::parser::make_DOT(); }

\#								{ return yy::parser::make_LENGTH(Node("LENGTH", yytext)); }

[ ]*							{ /* munch */ }
\n								{ /* munch */ }
\t								{ /* munch */ }

<<EOF>>							return yy::parser::make_END_OF_FILE();

%%