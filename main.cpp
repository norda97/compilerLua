#include <iostream>

#include "labTrees.cc"
#include "Utils.hpp"
#include "binary.tab.hh"
#include <map>

extern FILE *yyin;
extern Node root;
extern int processIndex;

// Init for expressions static nCounter
int Expression::nCounter = 0;

void yy::parser::error(std::string const&err)
{
	std::cout << "\nFailed at process index: " << processIndex << " " << err << std::endl;
}

int main(int argc, char **argv) 
{
    /* ------ Create AST ------ */
    if (argc == 2) {
		// Check for lua data type
		std::string fn(argv[1]);
		std::size_t found = fn.find(".lua");
		if (found == std::string::npos) {
			std::cout << "file is not of type lua" << std::endl;
			return -1;
		}

		//Set lexer input to file
		yyin = fopen(argv[1], "r");
		
		yy::parser parser;
		if(!parser.parse()) {
			fclose(yyin);
            //Create AST dot file
			UTILS::dumpAST(root);
	
			// Create new block and start on creating control flow 
			BBlock block;
			std::map<std::string, Statement*> procedures;
			std::list<BBlock*> blockCFGS;
			
			// Go through root and evaluate statements, than catch returned statment which is main cf
			BBlock* startBlock = new BBlock();
			root.evalStatement(procedures)->convert(startBlock);
			// Push main cf to list
			blockCFGS.push_front(startBlock);
			
			//Convert all procedures to block CFGS
			for(auto it : procedures) {
				startBlock = new BBlock();
				it.second->convert(startBlock);
				blockCFGS.push_back(startBlock);
			}

			//Dump CFG in console file
			UTILS::dumpCFG(&blockCFGS);
			//Create CFG dot file
			UTILS::dumpGraph(&blockCFGS);
			//Create hybrid file target.cc
			UTILS::dumpHybrid(&blockCFGS);

			for(auto it : blockCFGS) {
				delete it;
			}
		}
	}

	return 0;
}