#include "BasicBlock.hpp"

#include <fstream>

/* Basic Blocks */
int BBlock::nCounter = 0;

BBlock::BBlock() : tExit(NULL), fExit(NULL), name("block" + std::to_string(nCounter++)) {}

void BBlock::dump()
{
		std::cout << this->name << this << std::endl;
		for(auto i : instructions)
				i.dump();
		std::cout << "True:  " << tExit << std::endl;
		std::cout << "False: " << fExit << std::endl;
}

void BBlock::walkHybrid(std::ofstream& mfile) 
{
		int instructionSize = 0;
		bool isIfStatement = (tExit && fExit);
		if (isIfStatement) 
			instructionSize = instructions.size() - 1;
		else 
			instructionSize = instructions.size();

		
		mfile << this->name << ':' << '\n';

		auto it = instructions.begin();
		for(int i = 0; i < instructionSize; i++) {
			(it++)->expand(mfile);
		}
	
		if (isIfStatement) {
			//Comment
			mfile << "// Block: ";
			it->expandIf(mfile);
			mfile << this->fExit->name;
			mfile << " / ";
			mfile << this->tExit->name << " otherwise";
			mfile << '\n';

			// If statment
			mfile << "if(";
			mfile << it->lhs << ' ' << it->op << ' ' << it->rhs;
			mfile << ") goto " << this->tExit->name;
			mfile << "; else goto " << this->fExit->name;
			mfile << ";\n";
		}
		else if (this->tExit)
			mfile << "goto " << this->tExit->name << ";\n\n";

		if (tExit == nullptr && fExit  == nullptr) {
			mfile << "return 0;\n";
		}
		
}

/* Used to dump graphviz graph */
void BBlock::dumpGraph(std::ofstream& mfile) {
		std::string instructionList = "";
		int instructionSize = 0;
		bool isIfStatement = (tExit && fExit);
		if (isIfStatement) 
			instructionSize = instructions.size() - 1;
		else 
			instructionSize = instructions.size();

		auto it = instructions.begin();
		for(int i = 0; i < instructionSize; i++) {
			instructionList += it->name + " := ";
			instructionList += it->lhs + " ";
			// Add escape for HTML characters
			if (it->op == "<" || it->op == ">" || it->op == "<=")
				instructionList += "\\";
			instructionList += it->op + " ";
			// Remove double quotes on strings
			if (it->rhs.front() == '"' && it->rhs.back() == '"') {
				std::string temp = it->rhs;
				temp.pop_back();
				temp.erase(temp.begin());
				instructionList += temp;
			}
			else
				instructionList += it->rhs;

			instructionList += "\\l";
			it++;
		}
		mfile <<"BLK_" <<  this << " [label=\"";
		mfile << '{';
		mfile << this->name << " @ " << this << "\\n";
		mfile << "| Instructions: " << "\\l";
		mfile << instructionList;
		
		if (isIfStatement) {
			mfile << "| Condition:\\l";

			mfile << it->name + " := " + it->lhs + " ";
			if (it->op == "<" || it->op == ">" || it->op == "<=")
				mfile << "\\";
			mfile << it->op;
			mfile << " " + it->rhs << "\\l";
		}
		mfile << "}\"]\n" <<"\n";

		if (tExit) {
			mfile <<"BLK_" <<  this << " -> " <<"BLK_" <<  tExit << "\n";
		}
		if (fExit) {
			mfile <<"BLK_" <<  this << " -> " <<"BLK_" <<  fExit;
			// Add color label
			mfile <<" [" <<  "color=\"#12661d\" "<< "];"<< "\n";
		}
}

void BBlock::walkSymbolTable(std::set<BBlock*>* visited, std::map<std::string, ThreeAd>* symbolTable) {

		// Check if block already visited
		for(auto it : *visited)
			if (&(*it) == this)
				return;
		// Add to set of visited
		visited->insert(this);
		
		// Loop through instructions and try to find new symbols
		for(auto it : this->instructions) {
			// Only insert if pair is of types specified below
			if (symbolTable->find(it.name) == symbolTable->end()
			&& (
				it.nameType == Type::DBL
			||	it.nameType == Type::VAR
			||	it.nameType == Type::RET
			||	it.nameType == Type::DBL_PTR
			||	it.nameType == Type::TBL)
			) 
			{
				symbolTable->insert(std::make_pair(it.name, it));
			}
		}

		if (tExit) {
			tExit->walkSymbolTable(visited, symbolTable);
		}
		if (fExit) {
			fExit->walkSymbolTable(visited, symbolTable);
		}
}

