#include "Node.hpp"

#include <cmath>
#include <fstream>
#include <string>

#include "labTrees.cc"


/* ----------- NODE definitions -----------------*/
Node::Node(std::string t, std::string v) : tag(t), value(v) {}

Node::Node() {
	this->tag = "Uninitialised";
	this->value = "Uninitialised";
}

/* ----------- Parse tree and debug generation ---------------------------*/
void Node::dump(int depth) {
	for (int i = 0; i < depth; i++)
		std::cout << "--";
	std::cout << ">" << this->tag << ":" << value << std::endl;
	for (auto i = this->children.begin(); i != this->children.end(); i++)
		(*i).dump(depth+1);
}

void Node::dump(std::ofstream& mfile) {
	mfile << "Node_" << this << " [label=\"" << this->tag << "\\n" << this->value << "\"]\n";
	for (auto i = this->children.begin(); i != this->children.end(); i++) {
		(*i).dump(mfile);
		mfile <<"Node_" <<  this << " -> " <<"Node_" <<  &(*i) << "\n";
	}
}

Expression* Node::evalExpression() {
	Expression* expr = nullptr;
	
	Node& front = this->children.front();
	Node& back = this->children.back();

	// pass through
	if (this->tag == "args" || this->tag == "ELSE") {
		//Passthrough
		Node& front = this->children.front();
		expr = front.evalExpression();
	}
	else if (this->tag == "LENGTH")
		expr = new Length(front.evalExpression());
	else if (this->tag == "TRUE")
		expr = new Constant(1);
	else if (this->tag == "FALSE")
		expr = new Constant(0);
	else if (this->tag == "NUMBER")
		expr = new Constant(std::stod(this->value));
	else if (this->tag == "STRING")
		expr = new String(this->value);
	else if (this->tag == "var" || this->tag == "NAME")
		expr = new Variable(this->value);
	else if (this->tag == "access") 
		expr = new Variable(back.value);
	else if (this->tag == "binop") {
		expr = new Binop(front.evalExpression(), this->value, back.evalExpression());
	} else if (this->tag == "f_call") {
		expr = new ReadFCall(front.evalExpression(), back.evalExpression());
	} else if (this->tag == "indexing") {
		expr = new Load(front.evalExpression(), back.evalExpression());
	}

	return expr;
}

Statement* Node::evalStatement(std::map<std::string, Statement*>& procedures) {
	Statement* statement;
	Node& front = this->children.front();
	Node& back = this->children.back();

	if (this->tag == "chunk" || this->tag == "block") {
		statement = new Seq();
		for (auto it = this->children.begin(); it != this->children.end(); it++) {
			//If node is a FUNCTION dont add it to current cfg create a new instead
			if (it->tag == "FUNCTION") {
				// Check if this function is already declared
				Node& fName = it->children.front();
				if (procedures.find(fName.value) == procedures.end()) {
					procedures[fName.value] = it->evalStatement(procedures);
				}
				else
					std::cout << "Redefinition of function : " << front.value << std::endl; 
			}
			else
				dynamic_cast<Seq*>(statement)->addStatement(it->evalStatement(procedures));
		}
	}
	else if (this->tag == "f_call") {
		FunctionCall* fCall = new FunctionCall(front.evalExpression());
		for(auto arg : back.children)
			fCall->addArg(arg.evalExpression());
		statement = fCall;
	}
	else if (this->tag == "FUNCTION") {
		// Only one argument will work for functions
		Node& arglist = back.children.front();
		Node& block = back.children.back();
		
		Expression* expPointer = nullptr;
		if (arglist.children.size() > 0) {
			//Take first argument
			Node& arg = arglist.children.front();
			expPointer = arg.evalExpression();
		}
			statement = new Function(front.evalExpression(), expPointer, block.evalStatement(procedures));

	}
	else if (this->tag == "FOR") {
		auto it = this->children.begin();

		Node index = *(it++);
		Node val = *(it++);
		Node until = *(it++);
		Node step = *(it++);

		statement = new For(
			new Assignment(
							index.evalExpression(),
							val.evalExpression()
			),
			new Assignment(
							index.evalExpression(),
							new Binop(
									index.evalExpression(),
									"+",
									step.evalExpression()
							)
			),
			new Binop(
					index.evalExpression(), 
					"<=",
					until.evalExpression()
			),
			(it)->evalStatement(procedures)
		);
	}
	else if (this->tag == "RETURN") {
		Node& explist = front.children.front();
		statement = new Return(explist.evalExpression());
	}
	else if (this->tag == "REPEAT") {
		statement = new Repeat(back.evalExpression(), front.evalStatement(procedures));
	}
	else if (this->tag == "IF") {
		auto it = this->children.begin();

		Node cond = *(it++);
		Node block = *(it++);
		Statement* elseBlock = nullptr;
		// If else block exists set it else leave it as nullptr
		if (it != this->children.end()) {
			elseBlock = it->children.front().evalStatement(procedures);
		}

		statement = new If(
						cond.evalExpression(),
						block.evalStatement(procedures),
						elseBlock
		);
	}
	else if (this->tag == "EQUAL") {
		Node& front = this->children.front();
		Node& back = this->children.back();

		Equal* eq = new Equal();

		int tempValues = 0;
		std::string temp;
		// Iterate through variables and create temporary storages
		auto var = front.children.begin();
		for (auto arg : back.children) {
			// Create temporary value to hold 
			if (arg.tag == "fieldlist") {
				TblName* tableName = new TblName(var->value);
				Table* table = new Table(tableName, new TblIndex(arg.children.size()));
				
				int i = 1;
				for (auto field : arg.children)
					table->addField(new Store(tableName,
						new Constant(i++),
						field.evalExpression()
					));
				eq->addAssignment(table);
			}
			else {
				temp = "temp" + std::to_string(tempValues++);
				eq->addAssignment(new Assignment(new Variable(temp), arg.evalExpression()));
			}
			var++;
		}

		// Add temp variables to variables
		auto arg = back.children.begin();
		int assigned = 0;
		for (auto var : front.children) {
			if (arg->tag != "fieldlist") {
				// Assign variables with temporary values
				temp = "temp" + std::to_string(assigned++);
				
				// Special case if indexing is used then we store instead
				if (var.tag != "indexing")
					eq->addAssignment(new Assignment(var.evalExpression(), new Variable(temp)));
				else {
					auto list = var.children.front();
					auto index = var.children.back();
					eq->addAssignment(new Store(list.evalExpression(), index.evalExpression(), new Variable(temp)));
				}
			}
			arg++;
		}
		statement = eq;
	}
	return statement;
}
