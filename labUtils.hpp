#ifndef LABDECL_HPP
#define LABDECL_HPP

#include <set>
#include <list>
#include <initializer_list>
#include <string>

class ThreeAd
{
public:
		std::string name,lhs,rhs;
		char op;

		ThreeAd(std::string name, char op, std::string lhs, std::string rhs);
		
		void expand(std::ofstream& mfile);

		void dump();
private:
		void expandAdd(std::ofstream& mfile);
		void expandAssign(std::ofstream& mfile);
		void expandMul(std::ofstream& mfile);
};

class BBlock
{
private:
		static int nCounter;
public:
		std::list<ThreeAd> instructions;
		BBlock *tExit, *fExit;
		std::string name;

		BBlock();

		void dump();

		void dumpGraph(std::ofstream& mfile);
		void walkHybrid(std::ofstream& mfile);
		void walkSymbolTable(std::set<BBlock*>* visited, std::set<std::string>* symbolTable);
};

class Expression
{
protected:
		static int nCounter;
public:
		std::string name;

		Expression();
		virtual std::string makeNames();
		virtual std::string convert(BBlock*) = 0; // Lecture 8 / slide 12.
};



namespace UTILS {
	void dumpGraph(BBlock * start);
	void dumpCFG(BBlock * start);

	void dumpHybrid(BBlock * start);

	void createSymbolTable(BBlock * start, std::ofstream& mfile);
}


#endif