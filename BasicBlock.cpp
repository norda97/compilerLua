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
			mfile << "# Block: ";
			it->expandIf(mfile);
			mfile << this->fExit->name;
			mfile << " / ";
			mfile << this->tExit->name << " otherwise";
			mfile << '\n';
			
			std::string asm_cmp_inst = "";
			if (it->op == "==")
				asm_cmp_inst = "je";
			if (it->op == "<=")
				asm_cmp_inst = "jbe";
			if (it->op == ">=")
				asm_cmp_inst = "jae";
			else if (it->op == ">")
				asm_cmp_inst = "ja";
			else if (it->op == "<")
				asm_cmp_inst = "jb";

			INST_TWO_ARG(mfile, "movsd", it->rhs, "%xmm0");
			INST_TWO_ARG(mfile, "movsd", it->lhs, "%xmm1");
			INST_TWO_ARG(mfile, "ucomisd", "%xmm0", "%xmm1");
			INST_ONE_ARG(mfile, asm_cmp_inst, this->tExit->name);
			INST_ONE_ARG(mfile, "jmp", this->fExit->name);
		}
		else if (this->tExit)
			mfile << "jmp " << this->tExit->name << ";\n\n";

		if (!this->tExit && !this->fExit) {
			// Return 0 for success
			mfile << "\tmovq $0, %rax\n";
			mfile << "\tret\n";
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

void BBlock::walkSymbolTable(std::set<BBlock*>* visited, std::list<ThreeAd>* symbolTable, std::set<std::string>* globalSymbolTable) {

		// Check if block already visited
		for(auto it : *visited)
			if (&(*it) == this)
				return;
		// Add to set of visited
		visited->insert(this);
		
		// Loop through instructions and try to find new symbols
		for(auto it : this->instructions) {
			// Only insert if pair is of types specified below
			if (globalSymbolTable->find(it.name) == globalSymbolTable->end()
			&& (
				it.nameType == Type::DBL
			||	it.nameType == Type::VAR
			||	it.nameType == Type::RET
			||	it.nameType == Type::STR
			||	it.nameType == Type::TBL)
			) 
			{
				globalSymbolTable->insert(it.name);
				symbolTable->push_back(it);

				//Add all variable occurnces to push list
				if (it.nameType == Type::VAR)
						ThreeAd::PreservedVariables.push_front(it.name);
			}
		}

		if (tExit) {
			tExit->walkSymbolTable(visited, symbolTable, globalSymbolTable);
		}
		if (fExit) {
			fExit->walkSymbolTable(visited, symbolTable, globalSymbolTable);
		}
}

