#include "ThreeAdd.hpp"

#include <fstream>
#include <iomanip>

/* -----------------------Quad operations ------------------------- */
#define INLINE_MOVQ(lhs, rhs) "movq \%" << lhs << ",\t\%" << rhs

#define INLINE_ADDQ(lhs, rhs) "addq \%" << lhs << ",\t\%" << rhs
#define INLINE_MULQ(lhs) "mulq \%" << lhs

/* --------------Single Precision Double Operations---------------- */
#define INLINE_MOVSD(lhs, rhs) "movsd \%" << lhs << ",\t\%" << rhs

#define INLINE_ADDSD(lhs, rhs) "addsd \%" << lhs << ",\t\%" << rhs
#define INLINE_MULSD(lhs, rhs) "mulsd \%" << lhs << ",\t\%" << rhs
#define INLINE_DIVSD(lhs, rhs) "divsd \%" << lhs << ",\t\%" << rhs
#define INLINE_SUBSD(lhs, rhs) "subsd \%" << lhs << ",\t\%" << rhs

/* -------------------------I/O operations------------------------- */

/* ----------------------Inline Operations------------------------- */
#define INLINE_READ(str) "[" << str << "]"

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
				|| op == "<" || op == "<=" || op == ">" || op == "/" )
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
	mfile << this->name << " = " << this->lhs << this->op << this->rhs << ";\n";
}

void ThreeAd::expandTable(std::ofstream& mfile) 
{
	mfile << "// Table " << this->name << " initilized up where all symbols are being defined" << "\n";
}

void ThreeAd::expandStore(std::ofstream& mfile) 
{
	mfile << this->name << '[' << std::stoi(this->lhs) << ']' << " = " << this->rhs << ";\n";
}

void ThreeAd::expandLoad(std::ofstream& mfile) 
{
	mfile << this->name << " = (" << this->lhs << " + (int)(" << this->rhs << "- 1));\n";
}

void ThreeAd::expandLength(std::ofstream& mfile) 
{
	mfile << this->name << " = " << "sizeof(" << this->lhs << ") / sizeof(" << this->lhs << "[0]);\n";
}

void ThreeAd::expandMod(std::ofstream& mfile) 
{
	this->updateParameters();
	mfile << this->name << " = (int)" << this->lhs << this->op << " (int)" << this->rhs << ";\n";
}

void ThreeAd::expandPow(std::ofstream& mfile) 
{
	this->updateParameters();
	mfile << this->name << " = pow(" << this->lhs << ", " << this->rhs << ");";
}

void ThreeAd::expandAssign(std::ofstream& mfile) 
{
	this->updateParameters();
	mfile << this->name << " = " << this->lhs << ";\n";
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
		mfile << "printf(\""<< type << "\\n\"," << this->rhs << ");";
	}
	else if ( this->lhs == "read")  {
		if (this->rhs == "\"*number\"")
			mfile << "scanf(\""<< "%lf" << "\",&" << this->name << ");";
		else if (this->rhs == "\"*line\"")
			mfile << "scanf(\""<< "%s" << "\\n\",&" << this->name << ");";
	}
	else if ( this->lhs == "write")  {
		mfile << "printf(\""<< type << "\"," << this->rhs << ");";
	}
	else {
		mfile << this->lhs << "(" <<  this->rhs << ");\n";
		mfile << this->name << " = " << _RETURN << ";";
	}
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
	}
}