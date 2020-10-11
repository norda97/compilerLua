#include "Utils.hpp"

#include <iostream>
#include <fstream>

#include "Node.hpp"
#include "BasicBlock.hpp"
#include "ThreeAdd.hpp"

namespace UTILS {
	/*
		Starting function for dumping C/inline assembly file
	*/
	void dumpHybrid(std::list<BBlock*>* cfgs) {
		
		std::set<BBlock *> done, todo;

		// Get main block and remove it from list
		BBlock* mainBlock =  cfgs->front();
		cfgs->pop_front();
		std::set<std::string> symbolTable;

		std::ofstream mfile("target.s");
		if (mfile.is_open()) {
			mfile << ".data\n";

			// predefined strings for printf, scanf and write
			mfile << PREDEFINED_F_P_ONE << ":\t .string " << "\"%.1f\\n\"" << "\t # PREDEFINED\n";
			mfile << PREDEFINED_STR << ":\t .string " << "\"%s\\n\"" << "\t # PREDEFINED\n";
			mfile << PREDEFINED_LF << ":\t .string " << "\"%lf\"" << "\t # PREDEFINED\n";
			mfile << PREDEFINED_F_P_ONE_NL << ":\t .string " << "\"%.1f\"" << "\t # PREDEFINED\n";
			mfile << PREDEFINED_STR_NL << ":\t .string " << "\"%s\"" << "\t # PREDEFINED\n";

			createSymbolTable(mainBlock, &symbolTable, mfile);

			mfile << ".text\n";
			// Declare main
			mfile << ".globl main\n";
			mfile << "main:\n";
			
			// // Reserved return variable
			// mfile << "// Reserved return variable" << "\n";
			// mfile << "double " << _RETURN << " = 0;\n\n";

			// Go through Main 
			todo.insert(mainBlock);
			while(todo.size()>0)
			{
				// Pop an arbitrary element from todo set
				auto first = todo.begin();
				BBlock *next = *first;
				todo.erase(first);
				// Walk through current
				next->walkHybrid(mfile);
				done.insert(next);

				// Add children onto todo
				if(next->tExit!=NULL && done.find(next->tExit)==done.end())
					todo.insert(next->tExit);
				if(next->fExit!=NULL && done.find(next->fExit)==done.end())
					todo.insert(next->fExit);
			}
			
			// Declare functions
			for(BBlock* procedure : *cfgs) {
				mfile << ".data\n";
				createSymbolTable(procedure, &symbolTable, mfile);
				
				mfile << ".text\n";
				// Get first instruction which holds information about function then remove it
				ThreeAd fInstr = procedure->instructions.front();
				procedure->instructions.pop_front();
				/*
					Because argument always will be double in testcases we don't have to 
					create templated function or different functions depending on usage
				*/
				fInstr.expand(mfile);
				

				// Go through blocks
				todo.insert(procedure);
				while(todo.size()>0)
				{
					// Pop an arbitrary element from todo set
					auto first = todo.begin();
					BBlock *next = *first;
					todo.erase(first);
					// Walk through current
					next->walkHybrid(mfile);
					done.insert(next);

					// Add children onto todo
					if(next->tExit!=NULL && done.find(next->tExit)==done.end())
						todo.insert(next->tExit);
					if(next->fExit!=NULL && done.find(next->fExit)==done.end())
						todo.insert(next->fExit);
				}

				mfile.close();
			}
		}
		else std::cout << "Couldn't create Assembly file" << std::endl;
	}
	/*
		Create symbol table
	*/
	void createSymbolTable(BBlock* start, std::set<std::string>* symbolTable, std::ofstream& mfile) {

		std::set<BBlock*> visited;
		std::list<ThreeAd> ProcedureSymbolTable;
		start->walkSymbolTable(&visited, &ProcedureSymbolTable, symbolTable);
				
		if (symbolTable->size()) {
			Type symbolType;
			std::string symbolName;
			std::string symbolValue;

			for (const auto& it : ProcedureSymbolTable) {
				symbolType = it.nameType;
				symbolValue = it.rhs;
				symbolName = it.name;

				switch (symbolType) {
					case Type::DBL:
						mfile << symbolName << ":\t .double " << symbolValue << "\t # CONSTANT\n";
						break;
					case Type::VAR:
						mfile << symbolName << ":\t .double 0.0\t # VARIABLE\n";
						break;
					case Type::RET:
						mfile << symbolName << ":\t .double 0.0\t # RETURN VARIABLE\n";
						break;
					case Type::STR:
						mfile << symbolName << ":\t .string \"" << symbolValue << "\"\t # STRING\n";
						break;
					case Type::TBL:
						std::string size = std::to_string((std::stoi(symbolValue)+1)* sizeof(double)) ; // add one because first spot will store length of table
						mfile << symbolName << ":\t .zero " << size  << "\t # TABLE\n";
						break;
				};
			}
		}
	}
	
	/*
		Dump the cfgs in graph form
	*/
	void dumpGraph(std::list<BBlock*>* cfgs) {
		std::ofstream mfile("cfg.dot");
		if (mfile.is_open()) {
			mfile << "digraph {\n";
			mfile << "graph [bgcolor=black]\n";
			mfile << "edge [color=green]\n";
			mfile << "node [shape=record color=green fontcolor=green]\n";
			// Go through blocks
			// Iterate over each procedure
			for (BBlock* it : *cfgs) {
				std::set<BBlock *> done, todo;
				todo.insert(it);
				while(todo.size()>0)
				{
					// Pop an arbitrary element from todo set
					auto first = todo.begin();
					BBlock *next = *first;
					todo.erase(first);
					// Walk through current
					next->dumpGraph(mfile);
					done.insert(next);

					// Add children onto todo
					if(next->tExit!=NULL && done.find(next->tExit)==done.end())
						todo.insert(next->tExit);
					if(next->fExit!=NULL && done.find(next->fExit)==done.end())
						todo.insert(next->fExit);
				}
			}
			mfile << "}\n";
			mfile.close();
		}
		else std::cout << "Couldn't create .dot file" << std::endl;
	}

	/*
		Dump the AST in graph form
	*/
	void dumpAST(Node& root) {
		std::ofstream mfile("parse.dot");
		if (mfile.is_open()) {
			mfile << "digraph {\n";
			mfile << "graph [bgcolor=black]\n";
			mfile << "edge [color=green]\n";
			mfile << "node [color=green fontcolor=green]\n";
			root.dump(mfile);
			mfile << "}\n";
			mfile.close();
		}
		else std::cout << "Couldn't create file" << std::endl;
	}

	
	/*
	* Iterate over each basic block that can be reached from the entry point
	* exactly once, so that we can dump out the entire graph.
	* This is a concrete example of the graph-walk described in lecture 7.
	*/
	void dumpCFG(std::list<BBlock*>* cfgs)
	{
		// Iterate over each procedure
		for (BBlock* it : *cfgs) {
			std::set<BBlock *> done, todo;
			todo.insert(it);
			while(todo.size()>0)
			{
				// Pop an arbitrary element from todo set
				auto first = todo.begin();
				BBlock *next = *first;
				todo.erase(first);
				next->dump();
				done.insert(next);
				if(next->tExit!=NULL && done.find(next->tExit)==done.end())
						todo.insert(next->tExit);
				if(next->fExit!=NULL && done.find(next->fExit)==done.end())
						todo.insert(next->fExit);
			}
		}
	}
}

