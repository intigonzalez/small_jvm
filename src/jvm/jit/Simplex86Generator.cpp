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
	if (scope == Local || scope == Temporal) {
		return "[ebp-" + std::to_string(offsetInStack) + "]";
	}
	jit_value value = { type, scope, n };
	return value.toString();
}

void Variable::setRegisterLocation(CPURegister* r) {
	(*this) = r;
	selfStored = false;
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
	for (M2MRelationship<CPURegister*, Variable*>::iterator2 it =
			locations.begin1(this), itEnd = locations.end1(this);
			it != itEnd; ++it){
		Variable* v = (*it);
		if (!v->inVar() && v->inRegister(this)) {
			fn(v->toString(), name);
			v->selfStored = true;
		}
	}
	locations.removeAll1(this);
}

CPURegister* Simplex86Generator::ensureValueIsInRegister(const jit_value& op2, const Vars& vars, ulong fixed, bool generateMov) {
	Variable* v = vars.get(op2);
	if (v && v->inRegister())  {
		// the value is in a register
		return v->getRegisterWithValue();
	}
	bitset<6> toSkip(fixed);
	unsigned idx = 0;
	unsigned min = 100000;
	int imin = -1;
	while (idx < registers.size() && (registers[idx]->nrHoldedValues() > 0 || toSkip.test(idx))) {
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

CPURegister* Simplex86Generator::ensureValueIsInRegister(const jit_value& operand, const Vars& vars) {
	return ensureValueIsInRegister(operand, vars, 0, true);
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
			functor.S() << "mov " << registers[0]->name << "," << getData(operand, vars) << '\n';
		}
		registers[3]->freeRegister(functor); // Free edx
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
	operatorToInstruction[AND] = "and ";
	operatorToInstruction[OR] = "or ";
	operatorToInstruction[SAR] = "sar ";
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
		Variable* v1;
		CPURegister* reg;
		CPURegister* reg1;
		CPURegister* reg2;
		int int_value;
		void * pointer;

		void deattach(CPURegister* r);
		switch (ope) {
		case CRAZY_OP:
			reg = ensureValueIsInRegister(op1, variables);
			v = variables.get(op1);
			if (v && !v->inRegister(reg)) {
				(*v) = reg;
//				v->selfStored = true;
			}

			v = variables.get(res);

			(*v) = reg;
			v->selfStored = false;

			// FIXME: this seems to be redundant
//			functor.S() << "mov " << v->toString() << "," << reg->name << '\n';
			continue;
		case ASSIGN:
			if (op1.meta.scope == Constant) {
				v = variables.get(res);
				functor.S() << "mov dword " << v->toString() << "," << op1.toString()
						<< '\n';
				(*v) = 0;
			} else {
				// variable
				// find empty register
				reg = ensureValueIsInRegister(op1, variables);
				v = variables.get(res);
				functor.S() << "mov " << v->toString() << "," << reg->name << '\n';
				(*v) = reg;
			}
			v->selfStored = true;
			break;
		case PLUS:
		case MUL:
		case SUB:
		case SHL:
		case SHR:
		case AND:
		case OR:
		case SAR:
			if (op1.meta.scope != Constant || op2.meta.scope != Constant) {
				// find register for op1 and copy it if necessary
				reg = ensureValueIsInRegister(op1, variables);
				// Ok, something is wrong. Look at this case
				// Let's say that op1 was in a register and the value has
				// not been saved to the var. then, the value will never be
				// saved. Even more, it will map a wrong storage
				// location to the var. I am trying to fix it with the
				// sentence below
				v1 = variables.get(op1);
				if (op1.meta.scope == Temporal) // there is no point in saving this variable
					(*v1) -= reg;
				reg->freeRegister(functor);

			} else {
				/* both are constants */
				cout << ope << " FUCK " << op1.value << op2.value << endl;
				assert(false);
			}
			functor.S() << operatorToInstruction[ope] << reg->name << ","
					<< getData(op2, variables) << '\n';
			v = variables.get(res);
			(*v) = reg;
			v->selfStored = false;
			break;
		case DIV:
		case REM:
			if (op1.meta.scope != Constant || op2.meta.scope != Constant) {
				reg = getRegistersForDiv(op1, variables); // it is always eax
				// Ok, something is wrong. Look at this case
				// Let's say that op1 was in a register and the value has
				// not been saved to the var. the, the value will never be
				// saved because. Even more, it will map a wrong storage
				// location to the var. I am trying to fix it with the
				// sentence below
				v1 = variables.get(op1);
				if (op1.meta.scope == Temporal) // there is no point in saving this variable
					(*v1) -= reg;
				reg->freeRegister(functor);
			} else {
				/* both are constants */
				assert(false);
			}
			functor.S() << "xor edx, edx" << '\n';
			tmpStr = getDataForDiv(op2, variables);
			functor.S() << "idiv dword " << tmpStr << '\n';
			if (ope == REM)
				reg = registers[3];

			v = variables.get(res);
			(*v) = reg;
			v->selfStored = false;
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
			if (op1.meta.scope != Constant) {
				// find register for op1 and copy it if necessary
				reg = ensureValueIsInRegister(op1, variables);
				v = variables.get(op1);
				if (v && !v->inRegister(reg)) {
					(*v) = reg;
//					v->selfStored = true;
				}
			} else {
				/* both are constants */
				assert(false);
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
			if (res.meta.type != Void) {
				reg = registers[0];
				v = variables.get(res);
				(*v) = reg;
				v->selfStored = false;
			}
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

			if (res.meta.type != Void) {
				reg = registers[0];
				v = variables.get(res);
				(*v) = reg;
				v->selfStored = false;
			}
			break;
		case MOV_FROM_ADDR:
			used.reset();
			reg1 = ensureValueIsInRegister(op1, variables); // the address
			used.set(reg1->id);
			reg = ensureValueIsInRegister(res,variables, used.to_ulong(), false); // the result will be here
			if (res.meta.type == CharType) {
				functor.S() << "xor " << reg->name << "," << reg->name << '\n';
				functor.S() << "mov " <<  reg->name16b << ",[" << reg1->name << "]\n";
			}
			else if (res.meta.type == Byte) {
				functor.S() << "xor " << reg->name << "," << reg->name << '\n';
				functor.S() << "mov " <<  reg->name8b << ",[" << reg1->name << "]\n";
			}
			else
				functor.S() << "mov " <<  reg->name << ",[" << reg1->name << "]\n";
			v = variables.get(res);
			(*v) = reg;
			v->selfStored = false;
			break;
		case MOV_TO_ADDR:
			used.reset();
			reg1 = ensureValueIsInRegister(op1, variables); // the address to dereference
			used.set(reg1->id);
			reg = ensureValueIsInRegister(op2, variables, used.to_ulong(), true);
			if (op2.meta.type == CharType) {
				functor.S() << "mov [" <<  reg1->name << "]," << reg->name16b << "\n";
			}
			else if (op2.meta.type == Byte)
				functor.S() << "mov [" <<  reg1->name << "]," << reg->name8b << "\n";
			else
				functor.S() << "mov [" <<  reg1->name << "]," << reg->name << "\n";
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
				reg = ensureValueIsInRegister(op1, variables);
				if (reg->id != 0) {
					registers[0]->freeRegister(functor);
					functor.S() << "mov " << registers[0]->name << "," << reg->name << '\n';
				}
			}

			functor.S() << "pop edi\n";
			functor.S() << "pop esi\n";
			functor.S() << "pop ebx\n";

			functor.S() << "leave\n";
			functor.S() << "ret" << '\n';
			break;
		}
		if (op1.meta.scope == Temporal)
			(*variables.get(op1)) = 0;

		if (op2.meta.scope == Temporal)
			(*variables.get(op2)) = 0;
	}
	// FXIME : this is a Bottleneck for performance
	for (auto& r : registers)
		r->freeRegister(functor);
}

} /* namespace jit */
