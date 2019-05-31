#include "ThreeAdd.hpp"

#include <fstream>
#include <iomanip>

/* -----------------------Quad operations ------------------------- */
#define INLINE_MOVQ(lhs, rhs) "movq " << lhs << ",\t" << rhs
#define INLINE_MOVQ_IMM(lhs, rhs) "movq " << lhs << ",\t" << rhs

#define INLINE_ADDQ(lhs, rhs) "addq " << lhs << ",\t" << rhs
#define INLINE_MULQ(lhs) "mulq " << lhs
#define INLINE_DIVQ(lhs) "divq " << lhs

#define INLINE_PUSHQ(lhs) "pushq " << lhs
#define INLINE_POPQ(lhs) "popq " << lhs

#define INLINE_XORQ(lhs, rhs) "xorq " << lhs << ",\t" << rhs

/* -----------------------Misc operations ------------------------- */
#define INLINE_LEA(lhs, rhs) "lea " << lhs << ",\t" << rhs

/* --------------Single Precision Double Operations---------------- */
#define INLINE_INT_TO_DBL(lhs, rhs) "cvtsi2sdq " << lhs << ",\t" << rhs
#define INLINE_DBL_TO_INT(lhs, rhs) "cvttsd2siq " << lhs << ",\t" << rhs

#define INLINE_MOVSD(lhs, rhs) "movsd " << lhs << ",\t" << rhs

#define INLINE_ADDSD(lhs, rhs) "addsd " << lhs << ",\t" << rhs
#define INLINE_MULSD(lhs, rhs) "mulsd " << lhs << ",\t" << rhs
#define INLINE_DIVSD(lhs, rhs) "divsd " << lhs << ",\t" << rhs
#define INLINE_SUBSD(lhs, rhs) "subsd " << lhs << ",\t" << rhs

/* -------------------------I/O operations------------------------- */
#define INLINE_CALL(lhs) "call " << lhs

/* ----------------------Inline Operations------------------------- */
#define INLINE_READ(str) "%[" << str << "]"
#define INLINE_READ_NMOD(str) "[" << str << "]"

#define INLINE_STRING(str) '"' << str << '"'

#define INLINE_INPUT(mfile, lhs, op, rhs) mfile << '\t' << lhs << " \"" << op << "\" (" << rhs << ')' << '\n'
#define INLINE_OUTPUT(mfile, lhs, op, rhs) mfile << '\t' << lhs << "\t \"" << op << "\" (" << rhs << ')' << ",\n"
#define INLINE_OUTPUT_LAST(mfile, lhs, op, rhs) mfile << '\t' << lhs << "\t \"" << op << "\" (" << rhs << ')' << '\n'
#define INLINE_TOUCHED(mfile, str) mfile << '"' << str << "\","
#define INLINE_TOUCHED_LAST(mfile, str) mfile << '"' << str << '"'
#define INLINE_LINE(mfile, str) mfile << "\"\t" << str << "\\n\\t\"\n"

/************* Three Address Instructions *************/
ThreeAd::ThreeAd(std::pair<Type, std::string> name, std::string op, std::pair<Type, std::string> lhs, std::pair<Type, std::string> rhs) :
		name(name.second), op(op), lhs(lhs.second), rhs(rhs.second), nameType(name.first), lhsType(lhs.first), rhsType(rhs.first)
{}

void ThreeAd::dump()
{
		std::cout << name << " := ";
		std::cout << lhs << " " << op << " " << rhs << std::endl;
}

void ThreeAd::expand(std::ofstream& mfile)
{
		mfile << "//Expand:\t";
		mfile << this->name << " := ";
		mfile << this->lhs << " " << this->op << " " << this->rhs << "\n";

		// Inline assembly here!
		if (op == "ASSIGN")
			this->expandAssign(mfile);
		else if (op == "EQUALS")
			mfile << this->name << " = " << this->lhs << "==" << this->rhs << ";\n";
		else if (op == "TBL")
			this->expandTable(mfile);
		else if (op == "STORE")
			this->expandStore(mfile);
		else if (op == "LOAD")
			this->expandLoad(mfile);
		else if (op == "LENGTH")
			this->expandLength(mfile);
		else if (op == "*" || op == "+" || op == "-" || op == "/"
				|| op == "<" || op == "<=" || op == ">")
			this->expandBinop(mfile);
		else if (op == "%")
			this->expandMod(mfile);
		else if (op == "^")
			this->expandPow(mfile);
		else if (op == "CALL")
			this->expandFuncCall(mfile);
		else if (op == "FUNC")
			this->expandFunc(mfile);

		mfile << "\n";
}

void ThreeAd::expandFunc(std::ofstream& mfile) 
{
	mfile << "double " << this->name << "(";
	
	/*
		This course don't force us to
		look up how this function is used
		we always assume there should be a double
		if there is an argument
	*/
	if (this->rhsType != Type::NIL) 
			mfile << "double " << this->rhs;

	mfile << ")";
}

void ThreeAd::expandBinop(std::ofstream& mfile) 
{
	this->updateParameters();
	if ( op == "<" || op == "<=" || op == ">")
		mfile << this->name << " = " << this->lhs << this->op << this->rhs << ";\n";
	else {
		mfile << "asm(\n";
		INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("lhs"), "%%xmm0"));
		INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("rhs"), "%%xmm1"));

		if ( this->op  == "+")
			INLINE_LINE(mfile, INLINE_ADDSD("%%xmm1", "%%xmm0"));
		else if ( this->op  == "-")
			INLINE_LINE(mfile, INLINE_SUBSD("%%xmm1", "%%xmm0"));
		else if ( this->op  == "/")
			INLINE_LINE(mfile, INLINE_DIVSD("%%xmm1", "%%xmm0"));
		else if ( this->op  == "*")
			INLINE_LINE(mfile, INLINE_MULSD("%%xmm1", "%%xmm0"));

		INLINE_LINE(mfile, INLINE_MOVSD("%%xmm0", INLINE_READ(this->name)));

		mfile << ":";
		INLINE_INPUT(mfile, INLINE_READ_NMOD(this->name), "=x", this->name);
		mfile << ":";
		INLINE_OUTPUT(mfile, INLINE_READ_NMOD("lhs"), "x", this->lhs);
		INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("rhs"), "x", this->rhs);
		mfile << ":\t";
		INLINE_TOUCHED(mfile, "xmm0");
		INLINE_TOUCHED(mfile, "xmm1");
		INLINE_TOUCHED_LAST(mfile, "cc");
		mfile << "\n);\n";
	}
}

void ThreeAd::expandTable(std::ofstream& mfile) 
{
	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("list"), "%%rax"));
	INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("size"), "(%%rax)")); // First double in array is set to size of array
	mfile << ":\n";
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("list"), "g", this->name);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("size"), "x", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandStore(std::ofstream& mfile) 
{
	//mfile << this->name << '[' << std::stoi(this->lhs) << ']' << " = " << this->rhs << ";\n";
	
	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("tblPtr"), "%%rbx"));
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("offset"), "%%xmm0"));
	INLINE_LINE(mfile, INLINE_DBL_TO_INT("%%xmm0", "%%rax")); // Convert double to int
	INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("value"), "%%xmm0"));

	INLINE_LINE(mfile, INLINE_XORQ("%%rdx", "%%rdx")); // Zero high 64 bits
	INLINE_LINE(mfile, INLINE_MOVQ("$8", "%%rcx"));
	INLINE_LINE(mfile, INLINE_MULQ("%%rcx")); // Multiply offset with 8 bytes 

	INLINE_LINE(mfile, INLINE_MOVSD("%%xmm0", "(%%rbx, %%rax)")); // Add value to address value

	mfile << ":\n";
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("value"), "x", this->rhs);
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("tblPtr"), "g", this->name);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("offset"), "x", this->lhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED(mfile, "rcx");
	INLINE_TOUCHED(mfile, "rdx");
	INLINE_TOUCHED(mfile, "xmm0");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandLoad(std::ofstream& mfile) 
{
	//mfile << this->name << " = (" << this->lhs << " + (int)(" << this->rhs << "- 1));\n";
	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("tblPtr"), "%%rbx"));
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("offset"), "%%xmm0"));
	INLINE_LINE(mfile, INLINE_DBL_TO_INT("%%xmm0", "%%rax")); // Convert double to int

	INLINE_LINE(mfile, INLINE_XORQ("%%rdx", "%%rdx")); // Zero high 64 bits
	INLINE_LINE(mfile, INLINE_MOVQ("$8", "%%rcx"));
	INLINE_LINE(mfile, INLINE_MULQ("%%rcx")); // Multiply offset with 8 bytes 
	INLINE_LINE(mfile, INLINE_ADDQ("%%rax", "%%rbx")); // Add offset to list ptr

	INLINE_LINE(mfile, INLINE_MOVQ("%%rbx", INLINE_READ("dblPtr"))); // Add value to address value

	mfile << ":\n";
	INLINE_INPUT(mfile, INLINE_READ_NMOD("dblPtr"), "=g", this->name);
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("tblPtr"), "g", this->lhs);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("offset"), "x", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED(mfile, "rcx");
	INLINE_TOUCHED(mfile, "rdx");
	INLINE_TOUCHED(mfile, "xmm0");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandLength(std::ofstream& mfile) 
{
	//mfile << this->name << " = " << "sizeof(" << this->lhs << ") / sizeof(" << this->lhs << "[0]);\n";

	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("list"), "%%rax"));

	INLINE_LINE(mfile, INLINE_MOVSD("(%%rax)", INLINE_READ("size"))); // Add value to address value

	mfile << ":\n";
	INLINE_INPUT(mfile, INLINE_READ_NMOD("size"), "=x", this->name);
	mfile << ":";
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("list"), "g", this->lhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandMod(std::ofstream& mfile) 
{
	this->updateParameters();
	//mfile << this->name << " = (int)" << this->lhs << this->op << " (int)" << this->rhs << ";\n";

	mfile << "asm(\n";
	// Convert doubles to integers
	INLINE_LINE(mfile, INLINE_DBL_TO_INT(INLINE_READ("lhs"), "%%rax")); // Set bot 64 bits to täljare
	INLINE_LINE(mfile, INLINE_DBL_TO_INT(INLINE_READ("rhs"), "%%rbx")); // Set rbx to  divisor
	// Continue with modulo operation
	INLINE_LINE(mfile, INLINE_XORQ("%%rdx", "%%rdx")); // Zero top 64 bits 
	INLINE_LINE(mfile, INLINE_DIVQ("%%rbx"));
	INLINE_LINE(mfile, INLINE_INT_TO_DBL("%%rdx", INLINE_READ("remainder"))); // Return remainder

	// Set up
	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ_NMOD("remainder"), "=x", this->name);
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("lhs"), "x", this->lhs);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("rhs"), "x", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED(mfile, "rdx");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandPow(std::ofstream& mfile) 
{
	this->updateParameters();
	// mfile << this->name << " = pow(" << this->lhs << ", " << this->rhs << ");";
	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_PUSHQ("%%rbx"));

	INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("lhs"), "%%xmm0"));
	INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("rhs"), "%%xmm1"));
	INLINE_LINE(mfile, INLINE_CALL("pow"));

	INLINE_LINE(mfile, INLINE_POPQ("%%rbx"));

	INLINE_LINE(mfile, INLINE_MOVSD("%%xmm0", INLINE_READ(this->name)));

	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ_NMOD(this->name), "=x", this->name);
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("lhs"), "x", this->lhs);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("rhs"), "x", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "xmm0");
	INLINE_TOUCHED(mfile, "xmm1");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandAssign(std::ofstream& mfile) 
{
	this->updateParameters();
	//mfile << this->name << " = " << this->lhs << ";\n";

	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("lhs"), "%%xmm0"));
	INLINE_LINE(mfile, INLINE_MOVSD("%%xmm0", INLINE_READ("rhs")));

	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ_NMOD("rhs"), "=x", this->name);
	mfile << ":";
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("lhs"), "x", this->lhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "xmm0");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandFuncCall(std::ofstream& mfile) 
{
	std::string type;
	this->updateParameters();

	switch (this->rhsType) {
		case Type::STR:
				type = "%s";
			break;
		default:
				type = "%.1f";
			break;
	}

	if ( this->lhs == "print") {
		this->expandPrint(mfile);
	}
	else if ( this->lhs == "read")  {
		this->expandScanf(mfile);
	}
	else if ( this->lhs == "write")  {
		this->expandPrint(mfile, true);
	}
	else {
		// In the testcases there is always an input argument
		// mfile << this->lhs << "(" <<  this->rhs << ");\n";
		// mfile << this->name << " = " << _RETURN << ";";
		mfile << "asm(\n";
		INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("arg"), "%%xmm0"));

		INLINE_LINE(mfile, INLINE_PUSHQ("%%rbx")); // stack alignment

		INLINE_LINE(mfile, INLINE_CALL(this->lhs));
		
		INLINE_LINE(mfile, INLINE_POPQ("%%rbx"));

		mfile << ":\n";
		mfile << ":";
		INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("arg"), "x", this->rhs);
		mfile << ":\t";
		INLINE_TOUCHED(mfile, "xmm0");
		INLINE_TOUCHED_LAST(mfile, "cc");
		mfile << "\n);\n";
	}
}

void ThreeAd::expandPrint(std::ofstream& mfile, bool write) 
{
	std::string type = "";

	switch (this->rhsType) {
		case Type::STR:
				type = "%s";
			break;
		default:
				type = "%.1f";
			break;
	}

	if (!write)
		type += "\\n";
	
	mfile << "asm(\n";
	switch (this->rhsType) {
		case Type::STR:
				INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("arg1"), "%%rdi"));
				INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("arg2"), "%%rsi"));
				INLINE_LINE(mfile, INLINE_MOVQ_IMM("$0", "%%rax"));

				INLINE_LINE(mfile, INLINE_PUSHQ("%%r10"));
				INLINE_LINE(mfile, INLINE_PUSHQ("%%r11"));

				INLINE_LINE(mfile, INLINE_CALL("printf"));
				
				INLINE_LINE(mfile, INLINE_POPQ("%%r11"));
				INLINE_LINE(mfile, INLINE_POPQ("%%r10"));

				mfile << ":\n";
				mfile << ":";
				INLINE_OUTPUT(mfile, INLINE_READ_NMOD("arg1"), "g", INLINE_STRING(type));
				INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("arg2"), "g", this->rhs);
				mfile << ":\t";
				INLINE_TOUCHED(mfile, "rax");
				INLINE_TOUCHED(mfile, "rdi");
				INLINE_TOUCHED(mfile, "rsi");
				INLINE_TOUCHED(mfile, "rdx");
				INLINE_TOUCHED(mfile, "rcx");
				INLINE_TOUCHED(mfile, "rbx");
				INLINE_TOUCHED_LAST(mfile, "cc");
			break;
		default:
				INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("arg1"), "%%rdi"));
				INLINE_LINE(mfile, INLINE_MOVSD(INLINE_READ("arg2"), "%%xmm0"));
				INLINE_LINE(mfile, INLINE_MOVQ_IMM("$1", "%%rax"));
				
				INLINE_LINE(mfile, INLINE_PUSHQ("%%r10")); // Stack align forced to push two register, because one push dont gives me stackdump
				INLINE_LINE(mfile, INLINE_PUSHQ("%%r11")); //  This is wierd because this would align with 16 bytes, Dont know why this work

				INLINE_LINE(mfile, INLINE_CALL("printf"));
				
				INLINE_LINE(mfile, INLINE_POPQ("%%r11"));
				INLINE_LINE(mfile, INLINE_POPQ("%%r10"));

				mfile << ":\n";
				mfile << ":";
				INLINE_OUTPUT(mfile, INLINE_READ_NMOD("arg1"), "g",  INLINE_STRING(type));
				INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("arg2"), "x", this->rhs);
				mfile << ":\t";
				INLINE_TOUCHED(mfile, "rax");
				INLINE_TOUCHED(mfile, "rdi");
				INLINE_TOUCHED(mfile, "rsi");
				INLINE_TOUCHED(mfile, "rdx");
				INLINE_TOUCHED(mfile, "rcx");
				INLINE_TOUCHED(mfile, "rbx");
				INLINE_TOUCHED(mfile, "xmm0");
				INLINE_TOUCHED_LAST(mfile, "cc");
			break;
	}
	mfile << "\n);\n";


	//mfile << "printf(\""<< type << "\\n\"," << this->rhs << ");";
}

void ThreeAd::expandScanf(std::ofstream& mfile) 
{
	// if (this->rhs == "\"*number\"")
	// 	mfile << "scanf(\""<< "%lf" << "\",&" << this->name << ");";
	// else if (this->rhs == "\"*line\"")
	// 	mfile << "scanf(\""<< "%s" << "\\n\",&" << this->name << ");";

	mfile << "asm(\n";
	INLINE_LINE(mfile, INLINE_PUSHQ("%%rbx")); // stack align

	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("arg1"), "%%rdi"));
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("arg2"), "%%rsi"));

	INLINE_LINE(mfile, INLINE_CALL("scanf"));
	INLINE_LINE(mfile, INLINE_POPQ("%%rbx"));

	mfile << ":\n";
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ_NMOD("arg1"), "g",  INLINE_STRING("%lf"));
	INLINE_OUTPUT_LAST(mfile, INLINE_READ_NMOD("arg2"), "g", "&"+this->name);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rdi");
	INLINE_TOUCHED(mfile, "rsi");
	INLINE_TOUCHED(mfile, "rdx");
	INLINE_TOUCHED(mfile, "rcx");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED(mfile, "xmm0");
	INLINE_TOUCHED_LAST(mfile, "cc");
	mfile << "\n);\n";
}

void ThreeAd::expandIf(std::ofstream& mfile) 
{
	this->updateParameters();
	mfile << this->lhs + ' ' + this->op + ' '+ this->rhs << ' ';
}

void ThreeAd::updateParameters() 
{
	switch (this->nameType) {
		case Type::DBL_PTR:
				this->name = "*" + this->name;
			break;
	}
	switch (this->rhsType) {
		case Type::DBL_PTR:
				this->rhs = "*" + this->rhs;
			break;
		case Type::STR:
				this->rhs =  '"' + this->rhs + '"';
			break;
	}
	switch (this->lhsType) {
		case Type::DBL_PTR:
				this->lhs = "*" + this->lhs;
			break;
		case Type::STR:
				this->lhs =  '"' + this->lhs + '"';
			break;
	}
}