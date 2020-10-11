#ifndef THREE_ADD_HPP
#define THREE_ADD_HPP

#include <iostream>
#include <string>
#include <list>

#define _RETURN "_ret"

#define PREDEFINED_F_P_ONE "_arg_dbl"
#define PREDEFINED_F_P_ONE_NL "_arg_dbl_nl"
#define PREDEFINED_STR "_arg_str"
#define PREDEFINED_STR_NL "_arg_str_nl"
#define PREDEFINED_LF "_arg_long_float"

#define INST_ONE_ARG(mfile, instruction, arg) mfile << '\t' << instruction << ' ' << arg << "\n"
#define INST_TWO_ARG(mfile, instruction, lhs, rhs) mfile << '\t' << instruction << ' ' << lhs << ", " << rhs << "\n"

#define INT_TO_DBL(mfile,lhs, rhs) mfile << "\tcvtsi2sdq " << lhs << ",\t" << rhs << "\n"
#define DBL_TO_INT(mfile, lhs, rhs) mfile << "\tcvttsd2siq " << lhs << ",\t" << rhs << "\n"

enum Type {
	DBL,
	STR,
	VAR,
	TBL,
    FNC,
	RET,
	REG,
	NIL
};

class ThreeAd
{
public:
    std::string name,lhs,rhs, op;
    Type nameType, lhsType, rhsType;

    static std::list<std::string> PreservedVariables;

    ThreeAd(std::pair<Type, std::string> name, std::string op, std::pair<Type, std::string> lhs, std::pair<Type, std::string> rhs);
    
    void expand(std::ofstream& mfile);

    void expandIf(std::ofstream& mfile);

    void dump();
private:

    void expandAssign(std::ofstream& mfile);
    void expandBinop(std::ofstream& mfile);
    void expandMod(std::ofstream& mfile);
    void expandPow(std::ofstream& mfile);

    void expandPrint(std::ofstream& mfile, bool write = false);
    void expandScanf(std::ofstream& mfile);

    void expandLength(std::ofstream& mfile);

    void expandTable(std::ofstream& mfile);
    void expandStore(std::ofstream& mfile);
    void expandLoad(std::ofstream& mfile);

    void expandFuncCall(std::ofstream& mfile);
    void expandFunc(std::ofstream& mfile);
};

#endif