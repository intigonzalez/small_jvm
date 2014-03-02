/*
 * Quadru.h
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#ifndef QUADRU_H_
#define QUADRU_H_

#include "op_quadruplus.h"

#include <vector>
#include <string>
#include <iostream>
#include <set>

namespace jit {
enum value_scope {Constant, Local, Field, Temporal, Useless, Register, Label};
enum value_type {Integer, Byte, ObjRef,ArrRef, Void, CharType};

struct DataQuad {
	value_type op1_type:3;
	value_scope op1_scope:3;
	value_type op2_type:3;
	value_scope op2_scope:3;
	value_type op3_type:3;
	value_scope op3_scope:3;
	bool label:1;
	OP_QUAD op:13;
};

struct jit_value {
	struct {
		value_type type:3;
		value_scope scope:3;
	} meta;
	int32_t value;
	std::string toString() const {
		if (meta.scope == Constant) {
			return std::to_string(value);
		}
		else if (meta.scope == Useless)
			return "_";
		else if (meta.scope == Temporal) {
			return "T" + std::to_string(value);
		}
		else if (meta.scope == Local) {
			return "L" + std::to_string(value);
		}
		else if (meta.scope == Label) {
			return "LA" + std::to_string(value);
		}
		return "Wrong";
	}
} ;

extern jit_value  useless_value;

struct Quadr {
	OP_QUAD op;
	jit_value op1;
	jit_value op2;
	jit_value res;

	std::string toString() {
		std::string s;

		if (op == PUSH_ARG)
			s = pretty_printing[op] + (" " + op1.toString());
		else if (op == IINC) {
			s = op1.toString() + "<- " + op1.toString()
					+ pretty_printing[op] + op2.toString();
		}
		else if (op == CRAZY_OP) {
			s = res.toString() + "(<-) " + op1.toString();
		}
		else if (op >= PLUS && op <= SAR) {
			s = res.toString() + "<- " + op1.toString()
					+ pretty_printing[op] + op2.toString();
		}
		else if (op >= JGE && op <= JEQ) {
			s = "if " + op1.toString() + pretty_printing[op]
			                + op2.toString() + " goto " + res.toString();
		}
		else if (op == ASSIGN)
			s = res.toString() + "<- " + op1.toString();
		else if (op == MOV_FROM_ADDR)
			s = res.toString() + "<- (" + op1.toString() + ")";
		else if (op == MOV_TO_ADDR)
			s = "(" + op1.toString() + ")<- " + op2.toString();
		else if (op == OP_RETURN)
			s = pretty_printing[op] + (" " + op1.toString());
		else if (op == GOTO)
			s = pretty_printing[op] + (" " + res.toString());
		else {
			s += '(';
			s += pretty_printing[op];
			s += ",";
			s += op1.toString();
			s += ",";
			s += op2.toString() + "," + res.toString() + ")" + "\\n";
		}
		return s;
	}
};

struct BasicBlock {
	int label; // a basic block may have a label, this value is -1 if no label exist
	std::vector<Quadr> q;

	BasicBlock():label(-1) { }

	BasicBlock(int l):label(l) { }

	std::string getVizLabel() {
		std::string s("");
		s += "\\l";
		if (label != -1) {
			s += "LA";
			s += std::to_string(label);
			s += ":";
			s += "\\l";
		}
		for (unsigned i = 0; i < q.size(); ++i)
			s+= q[i].toString() + "\\l";
		return s;
	}
};

/**
 * Functions to deal with jit values
 */
jit_value jit_constant(int c);
jit_value jit_address(void* address);
jit_value jit_null();
jit_value jit_local_field(int index, value_type type);
jit_value jit_label(int pos);


}
#endif /* QUADRU_H_ */
