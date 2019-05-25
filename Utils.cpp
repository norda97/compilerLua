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

		std::ofstream mfile("target.cc");
		if (mfile.is_open()) {
			mfile << "#include<stdio.h>\n";
			mfile << "#include<math.h>\n";
			// Reserved return variable
			mfile << "// Reserved return variable" << "\n";
			mfile << "double " << _RETURN << " = 0;\n\n";

			// Declare functions
			for(BBlock* procedure : *cfgs) {
				// Get first instruction which holds information about function then remove it
				ThreeAd fInstr = procedure->instructions.front();
				procedure->instructions.pop_front();
				/*
					Because argument always will be double in testcases we don't have to 
					create templated function or different functions depending on usage
				*/
				fInstr.expand(mfile);
				mfile << "{\n";

				createSymbolTable(procedure, mfile);

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

				mfile << "}\n\n";
			}

			// Declare main

			createSymbolTable(mainBlock, mfile);
			mfile << "int main()\n{\n";
			
			// Go through blocks
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

			mfile << "}\n";
			mfile.close();
		}
		else std::cout << "Couldn't create Hybrid file" << std::endl;
	}
	/*
		Create symbol table
	*/
	void createSymbolTable(BBlock* start, std::ofstream& mfile) {

		std::set<BBlock*> visited;
		std::map<std::string, ThreeAd> symbolTable;
		start->walkSymbolTable(&visited, &symbolTable);
		
		if (symbolTable.size()) {
			const std::string separator = ", ";
			std::string sep1 = " ";
			std::string sep2 = " ";

			Type symbolType;
			std::string symbolName;

			bool tableExists = false;
			bool doubleExists = false;
			bool doublePtrExists = false;

			std::string doubleDeclartions = "double";
			std::string doublePtrDeclartions = "double";
			std::string tableDeclartions = "";

			for (const auto& it : symbolTable) {
				symbolType = it.second.nameType;
				symbolName = it.first;

				switch (symbolType) {
					case Type::DBL:
						doubleDeclartions += sep1 + symbolName + "=0";
						sep1 = separator;
						doubleExists = true;
						break;
					case Type::VAR:
						doubleDeclartions += sep1 + symbolName + "=0";
						sep1 = separator;
						break;
					case Type::DBL_PTR:
						doublePtrDeclartions += sep2 + "*" + symbolName + "=NULL";
						sep2 = separator;
						doublePtrExists = true;
						break;
					case Type::TBL:
						std::string size = std::to_string(std::stoi(it.second.rhs));
						tableDeclartions += "double " + symbolName + '[' + size + "];\n";
						tableExists =  true;
						break;
				};
				
			}

			if (doubleExists)
				mfile << doubleDeclartions << ";\n";
			if (doublePtrExists)
				mfile << doublePtrDeclartions << ";\n";
			if(tableExists)
				mfile << tableDeclartions << "\n";
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

