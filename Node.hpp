#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <list>	
#include <map>	

class Statement;
class Expression;

class Node {
public:

	Node(std::string t, std::string v);
	Node();

	void dump(int depth = 0);
	void dump(std::ofstream& mfile);
	
	Expression* evalExpression();
	// Evaluates new statments to procedures, if function definition is found a new procedure is added to procedures
	Statement* evalStatement(std::map<std::string, Statement*>& procedures);

	std::string tag, value;
	std::list<Node> children;
};

#endif