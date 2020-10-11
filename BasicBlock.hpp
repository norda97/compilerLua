#ifndef BASIC_BLOCK_HPP
#define BASIC_BLOCK_HPP

#include <iostream>
#include <list>
#include <set>
#include <map>

#include "ThreeAdd.hpp"

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
		void walkSymbolTable(std::set<BBlock*>* visited, std::list<ThreeAd>* symbolTable, std::set<std::string>* globalSymbolTable);
};


#endif