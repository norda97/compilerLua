#include "labUtils.hpp"

#include <iostream>
#include <fstream>


#define INLINE_MOVQ(lhs, rhs) "movq \%" << lhs << ",\t\%" << rhs

#define INLINE_ADDQ(lhs, rhs) "addq \%" << lhs << ",\t\%" << rhs
#define INLINE_MULQ(lhs) "mulq \%" << lhs

#define INLINE_READ(str) "[" << str << "]"

#define INLINE_INPUT(mfile, lhs, op, rhs) mfile << '\t' << lhs << " \"" << op << "\" (" << rhs << ')' << '\n'
#define INLINE_OUTPUT(mfile, lhs, op, rhs) mfile << '\t' << lhs << "\t \"" << op << "\" (" << rhs << ')' << ",\n"
#define INLINE_OUTPUT_LAST(mfile, lhs, op, rhs) mfile << '\t' << lhs << "\t \"" << op << "\" (" << rhs << ')' << '\n'
#define INLINE_TOUCHED(mfile, str) mfile << '"' << str << "\","
#define INLINE_TOUCHED_LAST(mfile, str) mfile << '"' << str << '"'
#define INLINE_LINE(mfile, str) mfile << "\"\t" << str << "\\n\\t\"\n"

/************* Three Address Instructions *************/
ThreeAd::ThreeAd(std::string name, char op, std::string lhs, std::string rhs) :
		name(name), op(op), lhs(lhs), rhs(rhs)
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
		mfile << "asm(\n";
		if (op == '+')
			this->expandAdd(mfile);
		else if (op == '*')
			this->expandMul(mfile);	
		else if (op == 'c')
			this->expandAssign(mfile);	

		mfile << "\n);\n";
		mfile << '\n';
}

void ThreeAd::expandAdd(std::ofstream& mfile) 
{
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("lhs"), "\%rax"));
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("rhs"), "\%rbx"));
	INLINE_LINE(mfile, INLINE_ADDQ("\%rbx", "\%rax"));
	INLINE_LINE(mfile, INLINE_MOVQ("\%rax", INLINE_READ(this->name)));

	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ(this->name), "=g", this->name);
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ("lhs"), "g", this->lhs);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ("rhs"), "g", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED_LAST(mfile, "cc");
}

void ThreeAd::expandMul(std::ofstream& mfile) 
{
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("lhs"), "\%rax"));
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("rhs"), "\%rbx"));
	INLINE_LINE(mfile, INLINE_MULQ("\%rbx"));
	INLINE_LINE(mfile, INLINE_MOVQ("\%rax", INLINE_READ(this->name)));

	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ(this->name), "=g", this->name);
	mfile << ":";
	INLINE_OUTPUT(mfile, INLINE_READ("lhs"), "g", this->lhs);
	INLINE_OUTPUT_LAST(mfile, INLINE_READ("rhs"), "g", this->rhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED(mfile, "rbx");
	INLINE_TOUCHED(mfile, "rdx");
	INLINE_TOUCHED_LAST(mfile, "cc");
}

void ThreeAd::expandAssign(std::ofstream& mfile) 
{
	INLINE_LINE(mfile, INLINE_MOVQ(INLINE_READ("lhs"), "\%rax"));
	INLINE_LINE(mfile, INLINE_MOVQ("\%rax", INLINE_READ(this->name)));

	mfile << ":";
	INLINE_INPUT(mfile, INLINE_READ(this->name), "=g", this->name);
	mfile << ":";
	INLINE_OUTPUT_LAST(mfile, INLINE_READ("lhs"), "g", this->lhs);
	mfile << ":\t";
	INLINE_TOUCHED(mfile, "rax");
	INLINE_TOUCHED_LAST(mfile, "cc");
}

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
			instructionList += it->lhs + " " + it->op + " " + it->rhs;
			instructionList += "\\l";
			it++;
		}
		mfile <<"BLK_" <<  this << " [label=\"";
		mfile << '{';
		mfile << "BLOCK @ " << this << "\\n";
		mfile << "| Instructions: " << "\\l";
		mfile << instructionList;
		
		if (isIfStatement) {
			mfile << "| Condition:\\l";
			mfile << it->name + " := " + it->lhs + " " + it->op + " " + it->rhs << "\\l";
		}
		mfile << "}\"]\n" <<"\n";

		if (tExit) {
			mfile <<"BLK_" <<  this << " -> " <<"BLK_" <<  tExit << "\n";
		}
		if (fExit) {
			mfile <<"BLK_" <<  this << " -> " <<"BLK_" <<  fExit << "\n";
		}
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
			mfile << "// Block: ";
			mfile << it->lhs + ' ' + it->op + ' '+ it->rhs << ' ';
			mfile << this->fExit->name;
			mfile << " / ";
			mfile << this->tExit->name << " otherwise";
			mfile << '\n';

			mfile << "if(";
			mfile << it->lhs + " == " + it->rhs;
			mfile << ") goto " << this->tExit->name;
			mfile << "; else goto " << this->fExit->name;
			mfile << ";\n";
		}
}

void BBlock::walkSymbolTable(std::set<BBlock*>* visited, std::set<std::string>* symbolTable) {

		// Check if block already visited
		for(auto it : *visited)
			if (&(*it) == this)
				return;
		// Add to set of visited
		visited->insert(this);
		
		// Loop through instructions and try to find new symbols
		for(auto it : this->instructions) {
			if (symbolTable->find(it.name) == symbolTable->end()) {
				symbolTable->insert(it.name);
			}
		}

		if (tExit) {
			tExit->walkSymbolTable(visited, symbolTable);
		}
		if (fExit) {
			fExit->walkSymbolTable(visited, symbolTable);
		}

}

/******************** Expressions ********************/
int Expression::nCounter = 0;

Expression::Expression() : name("") {}

std::string Expression::makeNames() {
	// Lecture 8 / slide 3-onwards.
	// Psuedo-code is illustrated on slide 5.
	// Virtual (but not pure) to allow overriding in the leaves.
	this->name = "_t" + std::to_string(nCounter++);
	return this->name;
}

namespace UTILS {
	void dumpGraph(BBlock* start) {

		std::ofstream mfile("cfg.dot");
		if (mfile.is_open()) {
			mfile << "digraph {\n";
			mfile << "graph [bgcolor=black]\n";
			mfile << "edge [color=green]\n";
			mfile << "node [shape=record color=green fontcolor=green]\n";
			// Go through blocks
			std::set<BBlock *> done, todo;
			todo.insert(start);
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
			mfile << "}\n";
			mfile.close();
		}
		else std::cout << "Couldn't create .dot file" << std::endl;
	}

	/*
	* Iterate over each basic block that can be reached from the entry point
	* exactly once, so that we can dump out the entire graph.
	* This is a concrete example of the graph-walk described in lecture 7.
	*/
	void dumpCFG(BBlock *start)
	{
			std::set<BBlock *> done, todo;
			todo.insert(start);
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

	/*
		Starting function for dumping C/inline assembly file
	*/
	void dumpHybrid(BBlock* start) {
		std::ofstream mfile("target.cc");
		if (mfile.is_open()) {
			mfile << "#include<iostream>\n";
			mfile << "int main()\n{\n";
			
			createSymbolTable(start, mfile);

			// Go through blocks
			std::set<BBlock *> done, todo;
			todo.insert(start);
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
		std::set<std::string> symbolTable;
		start->walkSymbolTable(&visited, &symbolTable);

		if (symbolTable.size()) {
			mfile << "long ";

			const char separator = ',';
			char sep = ' ';
			for(const auto& it : symbolTable) {
				mfile << sep << it << "=0";
				sep = separator;
			}

			mfile << ";\n";
		}
	}
}

