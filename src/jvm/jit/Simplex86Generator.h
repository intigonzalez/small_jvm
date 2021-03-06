/*
 * Simplex86Generator.h
 *
 *  Created on: Nov 1, 2013
 *      Author: inti
 */

#ifndef SIMPLEX86GENERATOR_H_
#define SIMPLEX86GENERATOR_H_



#include <string>
#include <set>
#include <fstream>

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../../utilities/TemporaryFile.h"
#include "../../utilities/Logger.h"
#include "../../utilities/ManyToMany.h"
#include "Quadru.h"
#include "Routine.h"

#include <stdint.h>


#include "../down_calls.h"

namespace jit {

class CPURegister;

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
	M2MRelationship<CPURegister*, Variable*>& locations;
public:
	value_type type;
	value_scope scope;
	int n;
	int offsetInStack;

	bool selfStored;

	Variable(value_scope s, int ind, M2MRelationship<CPURegister*, Variable*>& l) : locations(l) {
		type = Integer;
		scope = s;
		n = ind;
		id = n*100000 + scope;
		offsetInStack = 0;
		selfStored = (scope != Temporal); // fixme: wrong conditions. It should be "if it is a parameter"
	}

	Variable& operator=(CPURegister* r) {
		locations.removeAll2(this);
		if (r != nullptr) {
			locations.add(r, this);
		}
		return *this;
	}

	Variable& operator+=(CPURegister* r) {
		if (r != nullptr)
			locations.add(r, this);
		return *this;
	}

	Variable& operator-=(CPURegister* r) {
		if (r != nullptr)
			locations.remove(r, this);
		return *this;
	}

	void setRegisterLocation(CPURegister* r);

	bool inVar() {
		return selfStored; //this->valueIn.find(var) != var->valueIn.end();
	}

	bool inRegister(CPURegister* r) {
		return locations.contains(r, this);
	}

	bool inRegister() {
		return locations.contains2(this);
	}

	CPURegister* getRegisterWithValue() {
		if (inRegister())
			return (*locations.begin2(this));
		return nullptr;
	}

	std::string toString();
};

class Vars {
private:
	M2MRelationship<CPURegister*, Variable*>& locations;
public:
	std::set<Variable*, my_compare> variables;

	unsigned countOfParameters;
	int localCount;

	Vars(unsigned countOfParameters, M2MRelationship<CPURegister*, Variable*>& l):locations(l) {
		localCount = 0;
		this->countOfParameters = countOfParameters;
	}

	void addVariable(const jit_value& op) {
		if (op.meta.scope == Local || op.meta.scope == Temporal || op.meta.scope == Field) {
			Variable* v0 = new Variable(op.meta.scope, op.value, locations);
			v0->type = op.meta.type;
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

class CPURegister : public Identifiable {
private:
	M2MRelationship<CPURegister*, Variable*>& locations;
public:
	value_type type;
	std::string name;
	std::string name16b;
	std::string name8b;
	std::string name64b;

	CPURegister(const char* name, int number, M2MRelationship<CPURegister*, Variable*>& l) : locations(l) {
		this->name = name;
		this->name16b = this->name.substr(1); // FIXME, what a ugly hack. it works only for xi386
		std::string tmp = this->name16b.substr(0,1);
		this->name8b = tmp + "l"; // FIXME, even worst and it doesn't work with EDI and ESI
		this->name64b = name; this->name64b[0] = 'r';
		this->id = number;
		type = Integer;
	}

	template <class Function>
	void freeRegister(Function f);

	unsigned nrHoldedValues() {
		return locations.degree1(this);
	}
};

class Simplex86Generator {

public:
	Simplex86Generator();
	virtual ~Simplex86Generator();

	template <class CodeSectionManager>
	void* generate(Routine& routine, CodeSectionManager* manager);

	std::vector<void*> stubs2;

private:
	std::vector<CPURegister*> registers;
	ReleaseX86RegisterFunctor functor;
	M2MRelationship<CPURegister*, Variable*> m2mRegisterVariable;

	void generateBasicBlock(const Vars& variables, BasicBlock* bb);

	CPURegister* ensureValueIsInRegister(const jit_value& op2, const Vars& vars, ulong fixed, bool generateMov);
	CPURegister* ensureValueIsInRegister(const jit_value& operand, const Vars& vars);
	std::string getData(const jit_value& op2, const Vars& vars);
	CPURegister* getRegistersForDiv(const jit_value& operand, const Vars& vars);
	std::string getDataForDiv(const jit_value& operand, const Vars& vars);
};

template <class CodeSectionManager>
void* Simplex86Generator::generate(Routine& routine, CodeSectionManager* manager) {
	// definition of involved variables
	Vars variables(routine.countOfParameters, m2mRegisterVariable);
	for (auto& bb : routine.cfg)
		for (unsigned i = 0 ; i < bb->q.size(); ++i) {
			Quadr q = bb->q[i];
			variables.addVariable(q.op1);
			variables.addVariable(q.op2);
			variables.addVariable(q.res);
		}

	// init memory buffer
	void *buf = manager->nextAddress();
	// open file
	TemporaryFile file(".",".asm");
	file.open();
	functor = ReleaseX86RegisterFunctor(file.getFile());
	// let start generating
	functor.S() << "use64" << '\n';
	functor.S() << "ORG " << (uintptr_t)buf << '\n';
	functor.S() << "push rbp" << '\n';
	functor.S() << "mov rbp,rsp" << '\n';
	if (variables.variables.size() > 0) {
		LOG_DBG("NR Variables: " + to_string(variables.variables.size()));
		functor.S() << "sub rsp," << variables.variables.size()*4 << '\n';
	}

	functor.S() << "push rbx\n";
	functor.S() << "xor rax, rax\n";

	// TODO: let's remove the quad from the routine
//	routine.q.clear();

	// generate in order from the Control-Flow Graph
	// FIXME : I'm doing big assumptions here
	// 1 - I'm considering that every basic block will have at most two children
	// 2 - I'm considering that if the last instruction is a conditional jump then
	// 	   the second edge will be the jump and the first one will be the next
	//	   instruction (when the condition is false). This assumption holds for
	//	   my method to build the Control-Flow Graph but it is not general.
	std::vector<int> vec;
	int n = routine.cfg.nodes.size();
	bool* mark = new bool[n];
	std::fill(mark, std::next(mark, n), false);
	// for (int i = 0 ; i < n ; ++i)
		// mark[i] = false;
	vec.push_back(0);
	std::vector<int>::iterator it;
	while (!vec.empty()) {
		int v0 = vec.front(); vec.erase(vec.begin());
		mark[v0] = true;
		LOG_DBG("Compiling Block", v0);
		BasicBlock* bb = routine.cfg.nodes[v0];
		if (bb->label != -1)
			functor.S() << "LA" << bb->label << ":\n";
		generateBasicBlock(variables, bb);
		for (jvm::CFG::iteratorEdges ai = routine.cfg.outputs_begin(v0),
				ai_end = routine.cfg.outputs_end(v0);
				ai != ai_end ; ++ai) {
			int v1 = (*ai).trg;
			if (!mark[v1]) {
				it =std::find(vec.begin(), vec.end(), v1);
				if ((*ai).info == jvm::jmp_transition) {
					// it is a jump, no priority
					if (it == vec.end())
						vec.push_back(v1);
				}
				else {
					// it is coming now, needs priority
					if (it != vec.end()) {
						// it was inserted before but with low priority
						vec.erase(it);
					}
					it = vec.begin();
					vec.insert(it, v1);
				}
			}
		}
	}

	// add stub methods
	void* pointer = (void*)&loadClassCompileMethodAndPath;
	for (std::vector<void*>::iterator it = stubs2.begin(), itEnd = stubs2.end(); it != itEnd ; ++it) {
		void* address = *it;
		int n = std::distance(stubs2.begin(),it) + 100000;
		functor.S() << "LabelStub" << n << ": push dword " << address << '\n';
		functor.S() << "jmp " << pointer << '\n';
	}

	file.close();
	std::string output = file.getFilePath().substr(0, file.getFilePath().size() - 4) + ".bin";
	//std::string command = "nasm -f bin -o " + output + " " + file.getFilePath();
	std::string command = "fasm " + file.getFilePath()
			+ " " + output + " > /dev/null";
	LOG_DBG(command, " will execute");
	if (std::system(command.c_str())) {
		LOG_ERR("Error compiling method with back-end assembler");
		throw(std::runtime_error("Error compiling method"));
	}

	int fd2 = open(output.c_str(), O_RDONLY);
	buf = manager->getChunck(4096);
	int nbRead = read(fd2, buf, 4096);
	if (nbRead == 4096) {
		LOG_ERR("A bigger buffer is required to load the binary code");
		throw std::runtime_error("A bigger buffer is required to load the binary code");
	}
	close(fd2);

	return buf;
}

} /* namespace jit */
#endif /* SIMPLEX86GENERATOR_H_ */
