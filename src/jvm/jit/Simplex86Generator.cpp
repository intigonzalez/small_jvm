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
#include <fstream>
#include <cstdio>

#include "../down_calls.h"

using namespace std;

namespace jit {

std::string Variable::toString() {
	if (scope == Local || scope == Temporal)
		return "[ebp-" + std::to_string(offsetInStack) + "]";
	jit_value value = { type, scope, n };
	return value.toString();
}

void Variable::setSingleLocation() {
	for (auto& r : valueInR)
		r->deattachSimple(this);
	valueInR.clear();
	selfStored = true;
	needToBeSaved = false;
}

void Variable::setRegisterLocation(CPURegister* r) {
	for (auto& r : valueInR)
		r->deattachSimple(this);
	valueInR.clear();
	valueInR.insert(r);
	// FIXME : Iterate over variables
	selfStored = false;
	needToBeSaved = true;
}

void Variable::deattach(CPURegister* r) {
	valueInR.erase(r);
	r->deattachSimple(this);
}

void Variable::attach(CPURegister* r) {
	valueInR.insert(r);
	r->attachSimple(this);
}

Variable* Vars::get(const jit_value& value) const {
	set<Variable*>::iterator itV = std::find_if(variables.begin(), variables.end(), [value](Variable* vi){
		return vi->scope == value.meta.scope && vi->n == value.value;
	});
	if (itV == variables.end()) return nullptr;
	Variable* vv = *itV;
	if (value.meta.scope == Temporal) {
		int index = vv->n + localCount - countOfParameters ;
		vv->offsetInStack = 4+4*index;
	}
	else if (value.meta.scope == Local) {
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
void CPURegister::freeRegister(Function fn) {
	// save the register in every variable it knows
	for (auto& v : valueOf) {
		if (!v->inVar() && v->inRegister(this)) {
			v->deattachSimple(this);
			fn(v->toString(), name);
			v->selfStored = true;
		}
		else if (v->inVar() && v->inRegister(this)) {
			v->deattachSimple(this);
		}
	}
	valueOf.clear();
}

void CPURegister::setSingleReference(Variable* v) {
	for (auto& v : valueOf)
		v->deattachSimple(this);
	valueOf.clear();
	attach(v);
}

CPURegister* Simplex86Generator::getRegister(const jit_value& op2, const Vars& vars, ulong fixed, bool generateMov) {
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

CPURegister* Simplex86Generator::getRegister(const jit_value& operand, const Vars& vars) {
	return getRegister(operand, vars, 0, true);
}

std::string Simplex86Generator::getData(const jit_value& op2, const Vars& vars) {
	if (op2.meta.scope == Temporal || op2.meta.scope == Local || op2.meta.scope == Field) {
		Variable* v = vars.get(op2);
		if (v->inRegister())
			return v->getRegisterWithValue()->name;
		return v->toString();
	}
	return op2.toString();
}

CPURegister* Simplex86Generator::getRegistersForDiv(const jit_value& operand, const Vars& vars) {
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
	if (operand.meta.scope == Constant) {
		registers[1]->freeRegister(functor);
		functor.S() << "mov " << registers[1]->name << "," << operand.toString() << '\n';
		return registers[1]->name;
	}
	return getData(operand, vars);
}

static map<int, string> operatorToInstruction;

Simplex86Generator::Simplex86Generator() {
	registers.push_back(new CPURegister("eax",0, m2mRegisterVariable));
	registers.push_back(new CPURegister("ebx",1, m2mRegisterVariable));
	registers.push_back(new CPURegister("ecx",2, m2mRegisterVariable));
	registers.push_back(new CPURegister("edx",3, m2mRegisterVariable));
	registers.push_back(new CPURegister("esi",4, m2mRegisterVariable));
	registers.push_back(new CPURegister("edi",5, m2mRegisterVariable));

	operatorToInstruction[PLUS] = "add ";
	operatorToInstruction[SUB] = "sub ";
	operatorToInstruction[MUL] = "imul ";
	operatorToInstruction[SHL] = "shl ";
	operatorToInstruction[SHR] = "shr ";
	operatorToInstruction[JGE] = "jge ";
	operatorToInstruction[JG] = "jg ";
	operatorToInstruction[JLE] = "jle ";
	operatorToInstruction[JLT] = "jl ";
	operatorToInstruction[JEQ] = "jz ";
	operatorToInstruction[JNE] = "jne ";
}

Simplex86Generator::~Simplex86Generator() {
	for (auto& r : registers)
		delete r;
}

void Simplex86Generator::generateBasicBlock(const Vars& variables,
	BasicBlock* bb)
{
	string tmpStr;
	std::bitset<6> used;

	for (unsigned i = 0; i < bb->q.size(); i++) {
		if (bb->q[i].label != -1)
			functor.S() << "LA" << bb->q[i].label << ":\n";

		int ope = bb->q[i].op;
		jit_value op1 = bb->q[i].op1;
		jit_value op2 = bb->q[i].op2;
		jit_value res = bb->q[i].res;
		Variable* v;
		CPURegister* reg;
		CPURegister* reg1;
		CPURegister* reg2;
		int int_value;
		void * pointer;

		switch (ope) {
		case CRAZY_OP:
			reg = getRegister(op1, variables);
			v = variables.get(res);
			// FIXME: this seems to be redundant
			functor.S() << "mov " << v->toString() << "," << reg->name << '\n';
			v->setRegisterLocation(reg);
			reg->setSingleReference(v); // FIXME: I need many to many
			continue;
		case ASSIGN:
			if (op1.meta.scope == Constant) {
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
		case PLUS:
		case MUL:
		case SUB:
		case SHL:
		case SHR:
			if (op1.meta.scope != Constant || op2.meta.scope != Constant) {
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
		case DIV:
		case REM:
			if (op1.meta.scope != Constant || op2.meta.scope != Constant) {
				reg = getRegistersForDiv(op1, variables);
			} else {
				/* both are constants */
			}
			functor.S() << "xor edx, edx" << '\n';
			tmpStr = getDataForDiv(op2, variables);
			functor.S() << "idiv dword " << tmpStr << '\n';
			if (ope == REM)
				reg = registers[3];

			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case GET_ARRAY_POS:
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
				int_value = (op2.value << 2) + 12; // FIXME : size of the array element
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
		case SET_ARRAY_POS:
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
				int_value = (op2.value << 2) + 12; // FIXME : size of the array element
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
		case GOTO:
			// goto
			functor.S() << "jmp " << res.toString() << '\n';
			break;
		case IINC:
			// iinc
			v = variables.get(op1);
			functor.S() << "add dword " << v->toString() << "," << op2.toString()
					<< '\n';
			break;
		case JGE:
		case JLE:
		case JLT:
		case JG:
		case JNE:
		case JEQ:
			if (op1.meta.scope != Constant || op2.meta.scope != Constant) {
				// find register for op1 and copy it if necessary
				reg = getRegister(op1, variables);
			} else {
				/* both are constants */
			}
			functor.S() << "cmp " << reg->name << ","
					<< getData(op2, variables) << '\n';

			functor.S() << operatorToInstruction[ope] << res.toString() << '\n';
			break;
		case PUSH_ARG:
			if (op1.meta.type == Integer || op1.meta.type == ObjRef || op1.meta.type == ArrRef) {
				functor.S() << "push dword " << getData(op1, variables) << '\n';
			}
			// FIXME: other cases
			break;
		case PLAIN_CALL:
			// FIXME, take into account the return type
			registers[0]->freeRegister(functor);
			registers[2]->freeRegister(functor);
			registers[3]->freeRegister(functor);
			pointer = (void*)op1.value;
			functor.S() << "call dword " << pointer << '\n';
			if (op2.value > 0)
				functor.S() << "add esp, " << op2.value*4 << '\n'; // FIXME, number of parameters
			reg = registers[0];
			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case CALL_STATIC:
			// FIXME, take into account the return type
			registers[0]->freeRegister(functor);
			registers[2]->freeRegister(functor);
			registers[3]->freeRegister(functor);

			functor.S() << "mov ecx, LabelStub" << (stubs2.size() + 100000) << '\n';
			functor.S() << "call ecx\n";
			stubs2.push_back((void*)op1.value);

			if (op2.value > 0)
				functor.S() << "add esp, " << op2.value*4 << '\n'; // FIXME, number of parameters

			reg = registers[0];
			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case MOV_FROM_ADDR:
			used.reset();
			reg1 = getRegister(op1, variables); // the address
			used.set(reg1->id);
			reg = getRegister(res,variables, used.to_ulong(), false); // the result will be here
			functor.S() << "mov " <<  reg->name << ",[" << reg1->name << "]\n";
			v = variables.get(res);
			v->setRegisterLocation(reg);
			reg->setSingleReference(v);
			break;
		case MOV_TO_ADDR:
			used.reset();
			reg1 = getRegister(op1, variables); // the address to dereference
			used.set(reg1->id);
			reg = getRegister(op2, variables, used.to_ulong(), true);
//			reg = getRegister(res,variables); // the result will be here
			functor.S() << "mov dword [" <<  reg1->name << "]," << reg->name << "\n";
			break;
		case OP_RETURN:
			if (op1.meta.scope == Useless) {
				// do nothing, or maybe something with used registers
			}
			else if (op1.meta.scope == Constant) {
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
			functor.S() << "leave\n";
			functor.S() << "ret" << '\n';
			break;
		}
		if (op1.meta.scope == Temporal)
			(*variables.get(op1)) = 0;

		if (op2.meta.scope == Temporal)
			(*variables.get(op2)) = 0;
	}
	// FXIME : this is a BottleNect for performance
	for (auto& r : registers)
		r->freeRegister(functor);
}

} /* namespace jit */
