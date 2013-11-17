/*
 * Simplex86Generator.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: inti
 */

#include "Simplex86Generator.h"
#include "Quadru.h"

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <bitset>
#include <sstream>
#include <fstream>
#include <cstdio>

using namespace std;

namespace jit {

std::string Variable::toString() {
	std::ostringstream oss;
	if (scope == Local || scope == Temporal) {
		oss << "[ebp-" << offsetInStack << "]";
		return oss.str();
	}
	jit_value value = { type, scope, { n }};
	return value.toString();
}

void Variable::setSingleLocation() {
	for (auto& r : valueInR)
		r->deattachSimple(this);
	valueInR.clear();
	// FIXME : Iterate over variables
	valueIn.clear();
	this->attach(this);
	needToBeSaved = false;
}

void Variable::setRegisterLocation(x86Register* r) {
	for (auto& r : valueInR)
		r->deattachSimple(this);
	valueInR.clear();
	valueInR.insert(r);
	// FIXME : Iterate over variables
	valueIn.clear();
	needToBeSaved = true;
}

void Variable::markAsForgetten() {
	for (auto& r : valueInR)
		r->deattachSimple(this);
	valueInR.clear();
	// FIXME : Iterate over variables
	valueIn.clear();
}

void Variable::deattach(x86Register* r) {
	valueInR.erase(r);
	r->deattachSimple(this);
}

void Variable::attach(x86Register* r) {
	valueInR.insert(r);
	r->attachSimple(this);
}

Variable* Vars::get(const jit_value& value) const {
	Variable v(value.scope, value.value.constant);
	set<Variable*>::iterator itV = variables.find(&v);
	if (itV == variables.end()) return nullptr;
	Variable* vv = *itV;
	if (value.scope == Temporal) {
		int index = vv->n + localCount - countOfParameters ;
		vv->offsetInStack = 4+4*index;
	}
	else if (value.scope == Local) {
		if ((unsigned)vv->n >= countOfParameters) {
			int index = vv->n - countOfParameters;
			vv->offsetInStack = 4+4*index;
		}
		else {
			int index = vv->n;
			vv->offsetInStack = -8 - 4*index;
		}
	}
	return vv;
}

template <class Function>
void x86Register::freeRegister(Function fn) {
	// save the register in every variable it knows
	for (auto& v : valueOf) {
		if (!v->inVar(v) && v->inRegister(this)) {
			v->deattachSimple(this);
			fn(v->toString(), name);
//			ofile << "mov " << v->toString() << "," << name << '\n';
			v->attach(v);
		}
	}
	valueOf.clear();
}

void x86Register::setSingleReference(Variable* v) {
	for (auto& v : valueOf)
		v->deattachSimple(this);
	valueOf.clear();
	attach(v);
}

x86Register* Simplex86Generator::getRegister(const jit_value& op2, const Vars& vars, ulong fixed, bool generateMov) {
	Variable* v = vars.get(op2);
	if (v && v->inRegister())  {
		// the value is in a register
		return v->getRegisterWithValue();
	}
	bitset<6> toSkip(fixed);
	unsigned idx = 0;
	unsigned min = 100000;
	int imin = -1;
	while (idx < registers.size() && (registers[idx]->holdingValue() || toSkip.test(idx))) {
		if (registers[idx]->nrHoldedValues() < min && !toSkip.test(idx)) {
			imin = idx;
			min = registers[idx]->nrHoldedValues();
		}
		idx++;
	}
	if (idx == registers.size()) {
		// restore variables in imin
		registers[imin]->freeRegister(functor);
		idx = imin;
		if (imin == -1) {
			cerr << "No available register" << endl;
			return nullptr;
		}
	}

	if (generateMov)
		functor.S() << "mov " << registers[idx]->name << "," << ((v)?v->toString():op2.toString()) << '\n';
	return registers[idx];
}

x86Register* Simplex86Generator::getRegister(const jit_value& operand, const Vars& vars) {
	return getRegister(operand, vars, 0, true);
}

std::string Simplex86Generator::getData(const jit_value& op2, const Vars& vars) {
	if (op2.scope == Temporal || op2.scope == Local || op2.scope == Field) {
		Variable* v = vars.get(op2);
		if (v->inRegister())
			return v->getRegisterWithValue()->name;
		return v->toString();
	}
	return op2.toString();
}

x86Register* Simplex86Generator::getRegistersForDiv(const jit_value& operand, const Vars& vars) {
	Variable* v = vars.get(operand);
	if (v && v->inRegister())  {
		// the value is in a register
		if (!v->inRegister(registers[0])) {
			registers[0]->freeRegister(functor);
		}
		registers[3]->freeRegister(functor); // Free edx
		// FIXME : this is broken if the value is already in eax
		functor.S() << "mov " << registers[0]->name << "," << getData(operand, vars) << '\n';
		return registers[0];
	}
	registers[0]->freeRegister(functor);
	registers[3]->freeRegister(functor);
	functor.S() << "mov " << registers[0]->name << "," << ((v)?v->toString():operand.toString()) << '\n';
	return registers[0];
}

std::string Simplex86Generator::getDataForDiv(const jit_value& operand, const Vars& vars) {
	if (operand.scope == Constant) {
		registers[1]->freeRegister(functor);
		functor.S() << "mov " << registers[1]->name << "," << operand.toString() << '\n';
		return registers[1]->name;
	}
	return getData(operand, vars);
}

Simplex86Generator::Simplex86Generator() {
	registers.push_back(new x86Register("eax",0));
	registers.push_back(new x86Register("ebx",1));
	registers.push_back(new x86Register("ecx",2));
	registers.push_back(new x86Register("edx",3));
	registers.push_back(new x86Register("esi",4));
	registers.push_back(new x86Register("edi",5));
}

Simplex86Generator::~Simplex86Generator() {
	for (auto& r : registers)
		delete r;
}

void Simplex86Generator::generateBasicBlock(const Vars& variables,
	BasicBlock* bb) {
	map<int, string> operatorToInstruction;
	operatorToInstruction['+'] = "add ";
	operatorToInstruction['-'] = "sub ";
	operatorToInstruction['*'] = "imul ";
	for (unsigned i = 0; i < bb->q.size(); i++) {
		if (bb->q[i].label != -1)
			functor.S() << "LA" << bb->q[i].label << ":\n";

		int ope = bb->q[i].op;
		jit_value op1 = bb->q[i].op1;
		jit_value op2 = bb->q[i].op2;
		jit_value res = bb->q[i].res;
		Variable* v;
		x86Register* reg;
		x86Register* reg1;
		x86Register* reg2;
		string tmpStr;
		std::bitset<6> used;
		int int_value;
		switch (ope) {
		case '=':
			if (op1.scope == Constant) {
				v = variables.get(res);
				functor.S() << "mov dword " << v->toString() << "," << op1.toString()
						<< '\n';
				v->setSingleLocation();
			} else {
				// variable
				// find empty register
				reg = getRegister(op1, variables);
				v = variables.get(res);
				functor.S() << "mov " << v->toString() << "," << reg->name << '\n';
				v->setSingleLocation();
				reg->setSingleReference(v);
			}
			break;
		case '+':
		case '*':
		case '-':
			if (op1.scope != Constant || op2.scope != Constant) {
				// find register for op1 and copy it if necessary
				reg = getRegister(op1, variables);
			} else {
				/* both are constants */
			}
			functor.S() << operatorToInstruction[ope] << reg->name << ","
					<< getData(op2, variables) << '\n';
			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case '/':
		case '%':
			if (op1.scope != Constant || op2.scope != Constant) {
				reg = getRegistersForDiv(op1, variables);
			} else {
				/* both are constants */
			}
			functor.S() << "xor edx, edx" << '\n';
			tmpStr = getDataForDiv(op2, variables);
			functor.S() << "idiv dword " << tmpStr << '\n';
			if (ope == '%')
				reg = registers[3];

			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case '[':
			used.reset();
			// get register for array base
			v = variables.get(op1);
			reg = getRegister(op1, variables);
			used.set(reg->id);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			// get a different register for index
			v = variables.get(op2);
			if (v) {
				reg1 = getRegister(op2, variables, used.to_ulong(),true);
				used.set(reg1->id);
				v->setRegisterLocation(reg1);
				reg1->setSingleReference(v);
				int_value = -1;
			} else {
				int_value = (op2.value.constant << 2) + 12; // FIXME : size of the array element
			}
			// get a different register to store the values
			reg2 = getRegister(res, variables, used.to_ulong(),false);
			v = variables.get(res);
			v->setRegisterLocation(reg2);
			reg2->setSingleReference(v);
			if (int_value == -1)
				functor.S() << "mov " << reg2->name << ",[" << reg->name << "+4*"
						<< reg1->name << "+12]" << endl;
			else
				functor.S() << "mov " << reg2->name << ",[" << reg->name << "+"
						<< int_value << "]" << endl;

			break;
		case ']':
			// read from array and store in var
			used.reset();
			// get register for array base
			v = variables.get(res);
			reg = getRegister(res, variables);
			used.set(reg->id);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			// get a different register for index
			v = variables.get(op2);
			if (v) {
				reg1 = getRegister(op2, variables, used.to_ulong(),true);
				used.set(reg1->id);
				v->setRegisterLocation(reg1);
				reg1->setSingleReference(v);
				int_value = -1;
			} else {
				int_value = (op2.value.constant << 2) + 12; // FIXME : size of the array element
			}
			// get a different register to store the values
			v = variables.get(op1);
			if (v) {
				reg2 = getRegister(op1, variables, used.to_ulong(),true);
				v->setRegisterLocation(reg2);
				reg2->setSingleReference(v);
			}
			tmpStr = getData(op1, variables);
			if (int_value == -1)
				functor.S() << "mov dword " << "[" << reg->name << "+4*" << reg1->name
						<< "+12]," << tmpStr << endl;
			else
				functor.S() << "mov dword " << "[" << reg->name << "+" << int_value
						<< "]," << tmpStr << endl;

			//v = variables.get(op2);
			//ofile << "mov " << v->toString() << "," << reg2->name << endl;
			break;
		case 'L':
			// length of array
			used.reset();
			// find empty register
			v = variables.get(op1);
			reg = getRegister(op1, variables);
			used.set(reg->id);
			v->setSingleLocation();
			reg->setSingleReference(v);
			// get a different register to store the values
			reg2 = getRegister(res, variables, used.to_ulong(),false);
			v = variables.get(res);
			v->setRegisterLocation(reg2);
			reg2->setSingleReference(v);
			functor.S() << "mov " << reg2->name << ",[" << reg->name << "+8]\n";
			break;
		case 1:
			// goto
			functor.S() << "jmp " << res.toString() << '\n';
			break;
		case 2:
			// iinc
			v = variables.get(op1);
			functor.S() << "add dword " << v->toString() << "," << op2.toString()
					<< '\n';
			break;
		case 3:
		case 4:
		case 5:
			if (op1.scope != Constant || op2.scope != Constant) {
				// find register for op1 and copy it if necessary
				reg = getRegister(op1, variables);
			} else {
				/* both are constants */
			}
			functor.S() << "cmp " << reg->name << ","
					<< getData(op2, variables) << '\n';
			tmpStr = "jge ";
			if (ope == 4) tmpStr = "jle ";
			if (ope == 5) tmpStr = "jg ";

			functor.S() << tmpStr << res.toString() << '\n';
			break;
		case 'r':
			if (op1.scope == Constant) {
				registers[0]->freeRegister(functor);
				functor.S() << "mov " << registers[0]->name << "," << op1.toString()
						<< '\n';
			} else {
				// it is a variable
				reg = getRegister(op1, variables);
				if (reg->id != 0) {
					registers[0]->freeRegister(functor);
					functor.S() << "mov " << registers[0]->name << "," << reg->name << '\n';
				}
			}
			functor.S() << "add esp, " << variables.variables.size() * 4 << '\n';
			functor.S() << "pop ebp" << '\n';
			functor.S() << "ret" << '\n';
			break;
		}
		if (op1.scope == Temporal)
			variables.get(op1)->markAsForgetten();

		if (op2.scope == Temporal)
			variables.get(op2)->markAsForgetten();
	}
	for (auto& r : registers)
		r->freeRegister(functor);
}

} /* namespace jit */
