/*
 * Simplex86Generator.h
 *
 *  Created on: Nov 1, 2013
 *      Author: inti
 */

#ifndef SIMPLEX86GENERATOR_H_
#define SIMPLEX86GENERATOR_H_

#include "Quadru.h"

#include <string>
#include <set>

namespace jit {

class x86Register;

class Identifiable {
public:
	int id;
};

struct my_compare {
	bool operator() (const Identifiable* lhs, const Identifiable* rhs) const{
		return lhs->id < rhs->id;
	}
};

class Variable : public Identifiable {
private:
	std::set< Variable*, my_compare > valueIn; // set of variables where the value of this variables is.
	std::set< x86Register*, my_compare > valueInR; // set of registers where the variable is.
public:
	value_type type;
	value_scope scope;
	int n;
	bool needToBeSaved;
	int offsetInStack;

	Variable(value_scope s, int ind) {
		type = Integer;
		scope = s;
		n = ind;
		needToBeSaved = true;
		id = n*100000 + scope;
		offsetInStack = 0;
		if (scope != Temporal) valueIn.insert(this);
	}

	void setSingleLocation();

	void setRegisterLocation(x86Register* r);

	bool inVar(Variable* var) {
		return this->valueIn.find(var) != var->valueIn.end();
	}

	bool inRegister(x86Register* r) {
		return this->valueInR.find(r) != this->valueInR.end();
	}

	bool inRegister() {
		return !this->valueInR.empty();
	}

	x86Register* getRegisterWithValue() {
		if (inRegister())
			return (*this->valueInR.begin());
		return 0;
	}

	void markAsForgetten();

	std::string toString();

	void deattachSimple(x86Register* r) {
		valueInR.erase(r);
	}

	void deattach(x86Register* r);

	void attach(Variable* v) {
		valueIn.insert(v);
		//v->attachSimple(this);
	}

	void attachSimple(x86Register* r) {
		valueInR.insert(r);
	}

	void attach(x86Register* r);
};

class Vars {
public:
	std::set<Variable*, my_compare> variables;

	unsigned countOfParameters;
	int localCount;

	Vars(unsigned countOfParameters) {
		localCount = 0;
		this->countOfParameters = countOfParameters;
	}

	void addVariable(const jit_value& op) {
		if (op.scope == Local || op.scope == Temporal || op.scope == Field) {
			Variable* v0 = new Variable(op.scope, op.value.constant);
			v0->type = op.type;
			if (variables.find(v0) == variables.end() && v0->scope == Local)
				localCount ++;
			variables.insert(v0);
		}
	}

	Variable* get(jit_value& value) ;
};

class x86Register : public Identifiable {
private:
	std::set< Variable* , my_compare> valueOf; // set of variables whose values are in this register.
public:
	value_type type;
	const char* name;

	x86Register(const char* name, int number) {
		this->name = name;
		this->id = number;
		type = Integer;
	}

	void freeRegister();

	bool holdingValue() {
		return !this->valueOf.empty();
	}

	unsigned nrHoldedValues() {
		return this->valueOf.size();
	}

	void setSingleReference(Variable* v);

	void deattachSimple(Variable* v) {
		valueOf.erase(v);
	}

	void deattach(Variable* v) {
		valueOf.erase(v);
		v->deattachSimple(this);
	}

	void attachSimple(Variable* v) {
		valueOf.insert(v);
	}

	void attach(Variable* v) {
		valueOf.insert(v);
		v->attachSimple(this);
	}
};

class Simplex86Generator {
public:
	Simplex86Generator();
	virtual ~Simplex86Generator();

	void* generate(Routine& routine);

private:
	void generateBasicBlock(Vars variables, std::vector<x86Register*> registers, BasicBlock* bb);
};

} /* namespace jit */
#endif /* SIMPLEX86GENERATOR_H_ */
