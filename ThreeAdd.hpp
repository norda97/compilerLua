#ifndef THREE_ADD_HPP
#define THREE_ADD_HPP

#include <iostream>
#include <string>

#define _RETURN "_ret"

enum Type {
	DBL,
	DBL_PTR,
	STR,
	VAR,
	TBL,
    FNC,
	RET,
	NIL
};

class ThreeAd
{
public:
    std::string name,lhs,rhs, op;
    Type nameType, lhsType, rhsType;

    ThreeAd(std::pair<Type, std::string> name, std::string op, std::pair<Type, std::string> lhs, std::pair<Type, std::string> rhs);
    
    void expand(std::ofstream& mfile);

    void expandIf(std::ofstream& mfile);

    void dump();
private:
    void updateParameters();

    void expandAssign(std::ofstream& mfile);
    void expandBinop(std::ofstream& mfile);
    void expandMod(std::ofstream& mfile);
    void expandPow(std::ofstream& mfile);

    void expandPrint(std::ofstream& mfile);
    void expandWrite(std::ofstream& mfile);
    void expandScanf(std::ofstream& mfile);

    void expandLength(std::ofstream& mfile);

    void expandTable(std::ofstream& mfile);
    void expandStore(std::ofstream& mfile);
    void expandLoad(std::ofstream& mfile);

    void expandFuncCall(std::ofstream& mfile);
    void expandFunc(std::ofstream& mfile);
};

#endif