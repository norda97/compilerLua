#ifndef UTILS_HPP
#define UTILS_HPP

#include <set>
#include <list>
#include <map>

class BBlock;
class Node;

namespace UTILS {
	void dumpGraph(std::list<BBlock*> * cfgs);
	void dumpCFG(std::list<BBlock*>* cfgs);

	void dumpHybrid(std::list<BBlock*> * cfgs);

	void createSymbolTable(BBlock* start, std::ofstream& mfile);

	void dumpAST(Node& root);
}

#endif