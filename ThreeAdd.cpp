#include "ThreeAdd.hpp"

#include <fstream>
#include <iomanip>

//Initlize static list cotaning all touched variables during procedure
std::list<std::string> ThreeAd::PreservedVariables = std::list<std::string>();

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
		mfile << "# Expand:\t";
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
	mfile << this->name << ":";
}

void ThreeAd::expandBinop(std::ofstream& mfile) 
{

	if ( op == "<" || op == "<=" || op == ">")
		mfile << this->name << " = " << this->lhs << this->op << this->rhs << ";\n";
	else {
		INST_TWO_ARG(mfile, "movsd", this->lhs, "%xmm0");
		INST_TWO_ARG(mfile, "movsd", this->rhs, "%xmm1");

		if ( this->op  == "+")
			INST_TWO_ARG(mfile, "addsd", "%xmm1", "%xmm0");
		else if ( this->op  == "-")
			INST_TWO_ARG(mfile, "subsd", "%xmm1", "%xmm0");
		else if ( this->op  == "/")
			INST_TWO_ARG(mfile, "divsd", "%xmm1", "%xmm0");
		else if ( this->op  == "*")
			INST_TWO_ARG(mfile, "mulsd", "%xmm1", "%xmm0");

		INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name);
	}
}

void ThreeAd::expandTable(std::ofstream& mfile) 
{
	INST_TWO_ARG(mfile, "leaq", this->name, "%rax"); // Pointer to first element
	INST_TWO_ARG(mfile, "movq", '$'+this->rhs, "%rbx");
	INT_TO_DBL(mfile, "%rbx", "%xmm0");
	INST_TWO_ARG(mfile, "movsd", "%xmm0", "(%rax)");
}

void ThreeAd::expandStore(std::ofstream& mfile) 
{
	//mfile << this->name << '[' << std::stoi(this->lhs) << ']' << " = " << this->rhs << ";\n";
	INST_TWO_ARG(mfile, "leaq", this->name, "%rax"); // Pointer to first element
	INST_TWO_ARG(mfile, "movsd", this->lhs, "%xmm0"); // Offset index
	INST_TWO_ARG(mfile, "movsd", this->rhs, "%xmm1"); // Value
	DBL_TO_INT(mfile, "%xmm0", "%rbx"); // Convert double index to integer
	INST_TWO_ARG(mfile, "movsd", "%xmm1", "(%rax, %rbx, 8)");
}

void ThreeAd::expandLoad(std::ofstream& mfile) 
{
	INST_TWO_ARG(mfile, "leaq", this->lhs, "%rbx"); // Pointer to first element
	DBL_TO_INT(mfile, this->rhs, "%rax"); // Offset index
	INST_TWO_ARG(mfile, "movq", "(%rbx, %rax, 8)", "%rbx"); // Add offset
	INST_TWO_ARG(mfile, "movq", "%rbx", this->name); // move pointer to table index to variable
}

void ThreeAd::expandLength(std::ofstream& mfile) 
{
	//mfile << this->name << " = " << "sizeof(" << this->lhs << ") / sizeof(" << this->lhs << "[0]);\n";
	INST_TWO_ARG(mfile, "leaq", this->lhs, "%rax"); // Pointer to first element
	INST_TWO_ARG(mfile, "movsd", "(%rax)", "%xmm0"); 
	INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name); 
}

void ThreeAd::expandMod(std::ofstream& mfile) 
{
	//mfile << this->name << " = (int)" << this->lhs << this->op << " (int)" << this->rhs << ";\n";
	DBL_TO_INT(mfile, this->lhs, "%rax"); // Set bot 64 bits to täljare
	DBL_TO_INT(mfile, this->rhs, "%rbx"); // Set rbx to  divisor
	INST_TWO_ARG(mfile, "xorq", "%rdx", "%rdx"); // Zero top 64 bits 
	INST_ONE_ARG(mfile, "divq", "%rbx");
	INT_TO_DBL(mfile, "%rdx" ,"%xmm0");
	INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name);
}

void ThreeAd::expandPow(std::ofstream& mfile) 
{
	INST_ONE_ARG(mfile, "push", "%rbx # Alignment"); 

	INST_TWO_ARG(mfile, "movsd", this->lhs, "%xmm0");
	INST_TWO_ARG(mfile, "movsd", this->rhs, "%xmm1");
	INST_ONE_ARG(mfile, "call", "pow");
	INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name);

	INST_ONE_ARG(mfile, "pop", "%rbx"); 
}

void ThreeAd::expandAssign(std::ofstream& mfile) 
{
	
	//mfile << this->name << " = " << this->lhs << ";\n";
	INST_TWO_ARG(mfile, "movsd", this->lhs, "%xmm0");
	INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name);
}

void ThreeAd::expandFuncCall(std::ofstream& mfile) 
{
	std::string type;

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
		INST_TWO_ARG(mfile, "movsd", this->rhs, "%xmm0"); // Arg #1
		
		mfile << "# Preserve touched variable\n";
		for(auto touched : ThreeAd::PreservedVariables) {
			INST_ONE_ARG(mfile, "pushq", touched);
		}

		INST_ONE_ARG(mfile, "pushq", "%rbx"); // Align and preserve
		INST_ONE_ARG(mfile, "call", this->lhs);

		mfile << "# Restore all preserved variables\n";
		INST_ONE_ARG(mfile, "popq", "%rbx");
		// Go from back to front att pop
		auto it = ThreeAd::PreservedVariables.rbegin();
		while (it != ThreeAd::PreservedVariables.rend()) {
			INST_ONE_ARG(mfile, "popq", *(it++));
		}

		// Set return value
		INST_TWO_ARG(mfile, "movsd", _RETURN, "%xmm0"); 
		INST_TWO_ARG(mfile, "movsd", "%xmm0", this->name); // Save return value
	}
}

void ThreeAd::expandPrint(std::ofstream& mfile, bool write) 
{
	INST_ONE_ARG(mfile, "push", "%rbx # Alignment"); 
	switch (this->rhsType) {
		case Type::STR:
				write ? INST_TWO_ARG(mfile, "leaq", PREDEFINED_STR_NL, "%rdi")
				: INST_TWO_ARG(mfile, "leaq", PREDEFINED_STR, "%rdi");
				INST_TWO_ARG(mfile, "leaq", this->rhs, "%rsi");
				INST_TWO_ARG(mfile, "movq", "$0", "%rax");
				INST_ONE_ARG(mfile, "call", "printf");
			break;
		default:
				write ? INST_TWO_ARG(mfile, "leaq", PREDEFINED_F_P_ONE_NL, "%rdi")
				: INST_TWO_ARG(mfile, "leaq", PREDEFINED_F_P_ONE, "%rdi");
				INST_TWO_ARG(mfile, "movsd", this->rhs, "%xmm0");
				INST_TWO_ARG(mfile, "movq", "$1", "%rax");
				INST_ONE_ARG(mfile, "call", "printf");
			break;
	}
	INST_ONE_ARG(mfile, "pop", "%rbx"); 

	//mfile << "printf(\""<< type << "\\n\"," << this->rhs << ");";
}

void ThreeAd::expandScanf(std::ofstream& mfile) 
{
	// if (this->rhs == "\"*number\"")
	// 	mfile << "scanf(\""<< "%lf" << "\",&" << this->name << ");";
	// else if (this->rhs == "\"*line\"")
	// 	mfile << "scanf(\""<< "%s" << "\\n\",&" << this->name << ");";
 	INST_ONE_ARG(mfile, "push", "%rbx # Alignment"); 
	switch (rhsType) {
		case Type::STR:
			INST_TWO_ARG(mfile, "leaq", PREDEFINED_LF, "%rdi");
			INST_TWO_ARG(mfile, "leaq", this->name, "%rsi");
			INST_ONE_ARG(mfile, "call", "scanf");
			break;
		default:
			INST_TWO_ARG(mfile, "leaq", PREDEFINED_STR, "%rdi");
			INST_TWO_ARG(mfile, "leaq", this->name, "%rsi");
			INST_ONE_ARG(mfile, "call", "scanf");
			break;
	}
	INST_ONE_ARG(mfile, "pop", "%rbx"); 
}

void ThreeAd::expandIf(std::ofstream& mfile) 
{
	mfile << this->lhs + ' ' + this->op + ' '+ this->rhs << ' ';
}