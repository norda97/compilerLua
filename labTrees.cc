/* DV1465 / DV1505 / DV1511 Lab-task example code.
   (C) Dr Andrew Moss, Erik Bergenholtz  2016, 2017, 2018, 2019
   This code is released into the public domain.

   You are free to use this code as a base for your second assignment after
   the lab sessions (it is not required that you do so). 

   2018: Took out the double-pointers.
   2019: Clarified the comments, put the nCounter back in.
*/

#include <string>
#include <iostream>
#include "labUtils.hpp"

class Equality : public Expression
{
public:
		Expression *lhs, *rhs;

		Equality(Expression* lhs, Expression* rhs) :
				lhs(lhs), rhs(rhs)
		{
		}

		std::string convert(BBlock* out)
		{
			// Write three address instructions into the block
			lhs->convert(out);
			rhs->convert(out);

			std::string unique = this->makeNames();
			out->instructions.push_back(
				ThreeAd(unique, '=', lhs->name, rhs->name
			));

			return unique;
		}

};

class Add : public Expression
{
public:
		Expression *lhs, *rhs;

		Add(Expression* lhs, Expression* rhs) :
				lhs(lhs), rhs(rhs)
		{
		}

		std::string convert(BBlock* out)
		{
			// Write three address instructions into the block
			lhs->convert(out);
			rhs->convert(out);

			std::string unique = this->makeNames();
			out->instructions.push_back(
				ThreeAd(unique, '+', lhs->name, rhs->name
			));

			return unique;
		}

};

class Mult : public Expression
{
public:
		Expression *lhs, *rhs;

		Mult(Expression* lhs, Expression* rhs) :
				lhs(lhs), rhs(rhs)
		{
		}

		std::string convert(BBlock* out)
		{
			// Write three address instructions into the block
			lhs->convert(out);
			rhs->convert(out);
			std::string unique = this->makeNames();
			out->instructions.push_back(
				ThreeAd(unique, '*', lhs->name, rhs->name
			));

			return unique;
		}

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

		std::string convert(BBlock* out)
		{
				// Write three address instructions into the block
				return this->makeNames();
		}

		std::string makeNames() 
		{
			// Lecture 8 / slide 3-onwards.
			// Psuedo-code is illustrated on slide 5.
			// Virtual (but not pure) to allow overriding in the leaves.
			this->name = value;
			return this->name;
		}

};

class Constant : public Expression
{
public:
		int value;

		Constant(int value) :
				value(value)
		{
		}

		std::string convert(BBlock* out)
		{
				return this->makeNames();
		}

		std::string makeNames() 
		{
			// Lecture 8 / slide 3-onwards.
			// Psuedo-code is illustrated on slide 5.
			// Virtual (but not pure) to allow overriding in the leaves.
			this->name =  std::to_string(value);
			return this->name;
		}
};

/******************** Statements ********************/
class Statement
{
public:
		std::string name;

		Statement()
		{
		}
		virtual BBlock* convert(BBlock *) = 0;
};


class Seq : public Statement
{
public:
		std::list<Statement*> statments;
		Seq(std::initializer_list<Statement*> args)
		{
			statments = args;
		}

		BBlock* convert(BBlock *out)
		{
			for (Statement* stat : statments) {
				out = stat->convert(out);
			}

			return out;
		}
};

class Assignment : public Statement
{
public:
		Variable *lhs;
		Expression *rhs;

		Assignment(std::string lhs, Expression *rhs) :
				lhs(new Variable(lhs)), rhs(rhs)
		{
		}

		BBlock* convert(BBlock *out)
		{
			// Write three address instructions into the block
			lhs->convert(out);
			rhs->convert(out);

			out->instructions.push_back(
				ThreeAd(lhs->name, 'c', rhs->name, rhs->name
			));

			return out;
		}
};

class If : public Statement
{
public:
		Expression * eq;
		Statement *lhs;
		Statement *rhs;

		If(Expression* eq, Statement *lhs, Statement* rhs) :
				eq(eq), lhs(lhs), rhs(rhs)
		{
		}

		BBlock* convert(BBlock *out)
		{
			BBlock* t = new BBlock();
			BBlock* f = new BBlock();
			BBlock* j = new BBlock();

			eq->convert(out);

			BBlock* t2 = lhs->convert(t);
			BBlock* f2 = rhs->convert(f);

			t2->tExit = j;
			f2->tExit = j;

			out->tExit = t;
			out->fExit = f;

			return j;
		}
};


/* Test cases */
Statement *test = new Seq({
							new Assignment(
								  "x",
								  new Add(
										  new Variable("x"),
										  new Constant(1)
								  )
						  ),new If(
								  new Equality(
										  new Variable("x"),
										  new Constant(10)
								  ),new Assignment(
										  "y",
										  new Add(
												  new Variable("x"),
												  new Constant(1)
										  )
								  ), new Assignment(
										  "y",
										  new Mult(
												  new Variable("x"),
												  new Constant(2)
										  )
								  )
						  ), new Assignment(
								  "x",
								  new Add(
										  new Variable("x"),
										  new Constant(1)
								  )
						  )
});

int main() {
	BBlock *block = new BBlock();
	test->convert(block);
	UTILS::dumpCFG(block);
	UTILS::dumpGraph(block);
	UTILS::dumpHybrid(block);

	delete test;
	delete block;

	return 0;
}