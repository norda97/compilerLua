/* DV1465 / DV1505 / DV1511 Lab-task example code.
   (C) Dr Andrew Moss, Erik Bergenholtz  2016, 2017, 2018, 2019
   This code is released into the public domain.

   You are free to use this code as a base for your second assignment after
   the lab sessions (it is not required that you do so). 

   2018: Took out the double-pointers.
   2019: Clarified the comments, put the nCounter back in.
*/
#ifndef LAB_TREES_CPP
#define LAB_TREES_CPP

#include <string>
#include <iostream>
#include "BasicBlock.hpp"
#include "ThreeAdd.hpp"


#define CHANGE_PAIR_TYPE(pair, type) pair.first = type
#define ADD_INSTRUCTION(block, name, op, lhs, rhs) block->instructions.push_back(ThreeAd(name, op, lhs, rhs))


/******************** Expressions ********************/
class Expression
{
protected:
		static int nCounter;
public:
		std::string name;

		Expression() : name("") {};
		virtual ~Expression() {};
		std::pair<Type, std::string> makeNames(){
			this->name = "_t" + std::to_string(nCounter++);
			return std::make_pair(Type::VAR, this->name);
		}
		virtual std::pair<Type, std::string> convert(BBlock*) = 0; // Lecture 8 / slide 12.
};

class Variable : public Expression
{
public:
		std::string value;

		Variable(const std::string& value) :
				value(value)
		{
			this->name = value;
		}

		std::pair<Type, std::string> convert(BBlock* out)
		{
				// Write three address instructions into the block
				return std::make_pair(Type::VAR, value);
		}
};

class String : public Expression
{
public:
		std::string value;

		String(const std::string& value) :
				value(value)
		{
			this->name = value;
		}

		std::pair<Type, std::string> convert(BBlock* out)
		{
				// Write three address instructions into the block

				auto unique = this->makeNames();
				CHANGE_PAIR_TYPE(unique, Type::STR);
				auto pairValue = std::make_pair(Type::STR, value);
				ADD_INSTRUCTION(out, unique, "STR", pairValue, pairValue);

				return unique;
		}
};

class Constant : public Expression
{
public:
		double value;

		Constant(double value) : value(value) {}

		std::pair<Type, std::string> convert(BBlock* out)
		{
			auto unique = this->makeNames();
			CHANGE_PAIR_TYPE(unique, Type::DBL);
			auto pairValue = std::make_pair(Type::DBL,  std::to_string(value));
			ADD_INSTRUCTION(out, unique, "DBL", pairValue, pairValue);

			return unique;
		}
};

class Binop : public Expression
{
public:
		Expression *lhs, *rhs;
		std::string op;

		Binop(Expression* lhs, const std::string& op, Expression* rhs) :lhs(lhs), op(op), rhs(rhs) {}
		~Binop() {
			delete lhs;
			delete rhs;
		}
		std::pair<Type, std::string> convert(BBlock* out)
		{
			// Write three address instructions into the block
			auto lhsPair = lhs->convert(out);
			auto rhsPair = rhs->convert(out);

			auto unique = this->makeNames();
			ADD_INSTRUCTION(out, unique, op, lhsPair, rhsPair);

			return unique;
		}
};

class Load : public Expression
{
public:
		Expression *table, *index;

		Load(Expression *table, Expression *index) : table(table), index(index) {}
		~Load() {
			delete table;
			delete index;
		}

		std::pair<Type, std::string> convert(BBlock *out)
		{
			// // Write three address instructions into the block
			auto unique = this->makeNames();
			auto lhsPair = table->convert(out);
			auto rhsPair = index->convert(out);
			ADD_INSTRUCTION(out, unique, "LOAD", lhsPair, rhsPair);

			return unique;
		}
};


class ReadFCall : public Expression
{
public:
		Expression *lhs;
		Expression *rhs;

		ReadFCall(Expression* lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}
		~ReadFCall() 
		{
			delete lhs;
			delete rhs;
		}

		std::pair<Type, std::string> convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto lhsPair = lhs->convert(out);
			auto rhsPair = rhs->convert(out);

			auto unique = this->makeNames();
			ADD_INSTRUCTION(out, unique, "CALL", lhsPair, rhsPair);
			return unique;
		}
};

class Length : public Expression
{
public:
		Expression *lhs;

		Length(Expression *lhs) : lhs(lhs) {}
		~Length() {
			delete lhs;
		}

		std::pair<Type, std::string> convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto lhsPair = lhs->convert(out);
			auto unique = this->makeNames();
			ADD_INSTRUCTION(out, unique, "LENGTH", lhsPair, lhsPair);

			return unique;
		}
};


class TblName : public Expression
{
public:
		std::string value;

		TblName(const std::string& value) :
				value(value)
		{
			this->name = value;
		}

		std::pair<Type, std::string> convert(BBlock* out)
		{
				return std::make_pair(Type::STR, value);
		}
};

class TblIndex : public Expression
{
public:
		int value;

		TblIndex(int value) : value(value) {}

		std::pair<Type, std::string> convert(BBlock* out)
		{
			return std::make_pair(Type::DBL,  std::to_string(value));
		}
};

/******************** Statements ********************/
class Statement
{
public:
		std::string name;

		Statement() {};
		virtual ~Statement() {};
		virtual BBlock* convert(BBlock *) = 0;
};

class Seq : public Statement
{
public:
		std::list<Statement*> statments;
		Seq() {}
		~Seq() {
			for (auto it : statments)
				delete &(*it);
		}

		void addStatement(Statement* s) {
			this->statments.push_back(s);
		}

		BBlock* convert(BBlock *out)
		{	
			for (Statement* stat : statments)
				out = stat->convert(out);

			return out;
		}
};

class Assignment : public Statement
{
public:
		Expression *lhs;
		Expression *rhs;

		Assignment(Expression* lhs, Expression* rhs) : lhs(lhs), rhs(rhs) {}
		~Assignment() {
			delete lhs;
			delete rhs;
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto lhsPair = lhs->convert(out);
			auto rhsPair = rhs->convert(out);
			ADD_INSTRUCTION(out, lhsPair, "ASSIGN", rhsPair, rhsPair);
			return out;
		}
};

class Store : public Statement
{
public:
		Expression *table;
		Expression *index;
		Expression *value;

		Store(Expression* table, Expression* index, Expression* value) : table(table), index(index), value(value) {}
		~Store() {
			delete table;
			delete index;
			delete value;
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto tablePair = table->convert(out);
			auto indexPair = index->convert(out);
			auto valuePair = value->convert(out);
			ADD_INSTRUCTION(out, tablePair, "STORE", indexPair, valuePair);
			return out;
		}
};

class Return : public Statement
{
public:
		Expression *rhs;

		Return(Expression* rhs) : rhs(rhs) {}
		~Return() {
			delete rhs;
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto rhsPair = rhs->convert(out);

			ADD_INSTRUCTION(out, std::make_pair(Type::DBL, _RETURN), "ASSIGN", rhsPair, rhsPair);

			return out;
		}
};


class Table : public Statement
{
public:
		Expression *name, *length;
		std::list<Statement*> fields;

		Table(Expression *name,Expression *length) : name(name), length(length) {}
		~Table() {
			for (auto it : fields)
				delete it;
		}

		void addField(Statement * field) {
			this->fields.push_back(field);
		}

		BBlock*  convert(BBlock *out)
		{
			//Write three address instructions into the block
			auto lengthPair = length->convert(out);
			auto variablePair = name->convert(out);
			CHANGE_PAIR_TYPE(variablePair, Type::TBL);
			ADD_INSTRUCTION(out, variablePair, "TBL", lengthPair, lengthPair);

			for (auto it : fields) {
				it->convert(out);
			}

			return out;
		}
};

class Equal : public Statement
{
public:
		std::list<Statement*> assignments;

		Equal(){}
		~Equal() {
			for (auto it : assignments)
				delete it;
		}

		void addAssignment(Statement * assignment) {
			this->assignments.push_back(assignment);
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			for (auto it : assignments)
				it->convert(out);

			return out;
		}
};

class FunctionCall : public Statement
{
public:
		Expression *lhs;
		std::list<Expression*> args;

		FunctionCall(Expression* lhs) : lhs(lhs)  {}
		~FunctionCall() 
		{
			delete lhs;
			for (auto it : args)
				delete &(*it);
		}

		void addArg(Expression * arg) {
			this->args.push_back(arg);
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto lhsPair = lhs->convert(out);
			auto ret = std::make_pair(Type::RET, _RETURN);
			for (auto arg : args) {
				auto rhsPair = arg->convert(out);
				ADD_INSTRUCTION(out, ret, "CALL", lhsPair, rhsPair);
			}

			return out;
		}
};

class Function : public Statement
{
public:
		Expression *fname;
		Expression *arg;
		Statement *body;

		Function(Expression* fname, Expression *arg, Statement *body) : fname(fname),  arg(arg), body(body) {}
		~Function() 
		{
			delete fname;
			delete body;
			if (arg)
				delete arg;
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			auto fnamePair = fname->convert(out);
			CHANGE_PAIR_TYPE(fnamePair, Type::FNC);
			auto argPair = std::make_pair(Type::NIL, (std::string)"nil");
			auto regPair = std::make_pair(Type::REG, (std::string)"%xmm0");
			if(arg) 
 				argPair = arg->convert(out);

			ADD_INSTRUCTION(out, fnamePair, "FUNC", argPair, argPair);			
			ADD_INSTRUCTION(out, argPair, "ASSIGN", regPair, regPair);			
			return body->convert(out);
		}
};

class For : public Statement
{
public:
		Expression * cond;
		Statement *block, *initAssign, *continuesAssign;

		For(Statement *initAssign, Statement *continuesAssign, Expression *cond, Statement *block) :
				initAssign(initAssign), continuesAssign(continuesAssign), cond(cond), block(block)
		{}

		~For() 
		{
			delete initAssign;
			delete continuesAssign;
			delete cond;
			delete block;
		}

		BBlock* convert(BBlock *out)
		{
			BBlock* condBlock = new BBlock();
			BBlock* forBlock = new BBlock();
			BBlock* j = new BBlock();
			// Add instruction to loop block
			BBlock* innerJ = block->convert(forBlock);

			//connect loop exit to condition-block and add iteration inside block
			continuesAssign->convert(innerJ);
			innerJ->tExit = condBlock;
			
			// Connect condition-block to forBlock and continue block 
			condBlock->tExit = forBlock;
			condBlock->fExit = j;

			// Make addition before going into condition-block 
			initAssign->convert(out);

			// Add instructions to condition-block 
			cond->convert(condBlock);
			
			out->tExit = condBlock;

			return j;
		}
};

class Repeat : public Statement
{
public:
		Expression * cond;
		Statement *block;

		Repeat(Expression *cond, Statement *block) :
				cond(cond), block(block)
		{}

		~Repeat() 
		{
			delete cond;
			delete block;
		}

		BBlock* convert(BBlock *out)
		{
			BBlock* condBlock = new BBlock();
			BBlock* repeat = new BBlock();
			BBlock* j = new BBlock();
			// Add instruction to loop block
			BBlock* innerJ = block->convert(repeat);
			
			// Add instructions to condition-block
			cond->convert(condBlock);

			//connect loop exit to for-block
			innerJ->tExit = condBlock;
			
			// Connect for-block to loop and continue block 
			condBlock->tExit = j;
			condBlock->fExit = repeat;
			
			out->tExit = repeat;

			return j;
		}
};

class If : public Statement
{
public:
		Expression * eq;
		Statement *lhs;
		Statement *rhs;

		If(Expression* eq, Statement *lhs, Statement* rhs) : eq(eq), lhs(lhs), rhs(rhs) {}

		BBlock* convert(BBlock *out)
		{
			BBlock* t = new BBlock();
			BBlock* j = new BBlock();

			eq->convert(out);

			BBlock* t2 = lhs->convert(t);
			t2->tExit = j;
			out->tExit = t;
			// If rhs is nullptr no else exists
			if (rhs) 
			{
				BBlock* f = new BBlock();
				BBlock* f2 = rhs->convert(f);
				f2->tExit = j;
				out->fExit = f;
			}
			else {
				out->fExit = j;
			}
			
			return j;
		}
};

#endif