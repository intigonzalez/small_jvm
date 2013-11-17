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
#include <fstream>

#include <set>
#include <map>
#include <vector>
#include <iostream>

#include "../../utilities/TemporaryFile.h"

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

	Variable* get(const jit_value& value) const ;
};

class ReleaseX86RegisterFunctor {
private:
	std::ostream* stream;

public:
	ReleaseX86RegisterFunctor(std::ostream& s) :stream(&s) {}
	ReleaseX86RegisterFunctor() :stream(nullptr) {}
	void operator()(std::string var, std::string reg) {
		(*stream) << "mov " << var << "," << reg << '\n';
	}

	std::ostream& S() {
		return (*stream);
	}
};

class x86Register : public Identifiable {
private:
	std::set< Variable* , my_compare> valueOf; // set of variables whose values are in this register.
public:
	value_type type;
	std::string name;

	x86Register(const char* name, int number) {
		this->name = name;
		this->id = number;
		type = Integer;
	}

	template <class Function>
	void freeRegister(Function f);

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

	template <class CodeSectionManager>
	void* generate(Routine& routine, CodeSectionManager& manager);

private:
	std::vector<x86Register*> registers;
	ReleaseX86RegisterFunctor functor;

	void generateBasicBlock(const Vars& variables, BasicBlock* bb);

	x86Register* getRegister(const jit_value& op2, const Vars& vars, ulong fixed, bool generateMov);
	x86Register* getRegister(const jit_value& operand, const Vars& vars);
	std::string getData(const jit_value& op2, const Vars& vars);
	x86Register* getRegistersForDiv(const jit_value& operand, const Vars& vars);
	std::string getDataForDiv(const jit_value& operand, const Vars& vars);
};

template <class CodeSectionManager>
void* Simplex86Generator::generate(Routine& routine, CodeSectionManager& manager) {
	// definition of involved variables
	Vars variables(routine.countOfParameters);
	for (unsigned i = 0 ; i < routine.q.size(); i++) {
		Quadr q = routine.q[i];
		variables.addVariable(q.op1);
		variables.addVariable(q.op2);
		variables.addVariable(q.res);
	}
	// init memory buffer

	void *buf = manager.nextAddress();
	// open file
	TemporaryFile file(".",".asm");
	file.open();
	functor = ReleaseX86RegisterFunctor(file.getFile());
	// let start generating
	functor.S() << "BITS 32" << '\n';
	functor.S() << "ORG " << (unsigned)buf << '\n';
	functor.S() << "push ebp" << '\n';
	functor.S() << "mov ebp,esp" << '\n';
	functor.S() << "sub esp," << variables.variables.size()*4 << '\n';

	// let remove the quad from the routine
	routine.q.clear();

	// generate in order from the Control-Flow Graph
	// FIXME : I'm doing big assumptions here
	// 1 - I'm considering that every basic block will have at most to children
	// 2 - I'm considering that if the last instruction is conditional jump then
	// 	   the second edge will be the jump and the first one will be the next
	//	   instruction (when the condition is false). This assumption holds for
	//	   my method to build the Control-Flow Graph but it is not general.
	std::vector<vertex_t> vec;
	int n = boost::num_vertices(routine.g);
	bool* mark = new bool[n];
	for (int i = 0 ; i < n ; ++i) mark[i] = false;
	mark[0] = true;
	vec.push_back(0);
	while (!vec.empty()) {
		boost::graph_traits<ControlFlowGraph>::out_edge_iterator ai,ai_end;
		vertex_t v0 = vec.back();
		std::cout << " Block " << v0 << std::endl;
		vec.pop_back();
		BasicBlock* bb = routine.g[v0];
		generateBasicBlock(variables, bb);
		for (tie(ai, ai_end) = boost::out_edges(v0, routine.g) ; ai != ai_end ; ++ai) {
			if (!mark[(*ai).m_target]) {
				vec.push_back((*ai).m_target);
				mark[(*ai).m_target] = true;
			}
		}
	}

	file.close();
	std::string output = file.getFilePath().substr(0, file.getFilePath().size() - 4) + ".bin";
	std::string command = "nasm -f bin -o " + output + " " + file.getFilePath();
	std::cout << command << std::endl;
	int status = system(command.c_str());

	std::ifstream file2 (output.c_str(), std::ios::in|std::ios::binary);
	buf = manager.getChunck(4096);
	file2.read((char*)buf, 4096);
	file2.close();

	return buf;
}

} /* namespace jit */
#endif /* SIMPLEX86GENERATOR_H_ */
