/*
 * Quadru.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#include "Quadru.h"
#include <iostream>
#include <string>

#include <boost/graph/graphviz.hpp>

#include "../../utilities/Logger.h"
#include "Routine.h"

using namespace std;

namespace jit {

const char* pretty_printing[] = {
		"+", // PLUS
		"-", // SUB
		"*", // MUL
		"/", // DIV
		"%", // REM
		"+", // IINC
		"<<", // shl
		">>", // shr
		"&", // and
		"|", // or
		"sar", // sar
		"<-", // assign
		"mov_from_addr", // mov_from_addr
		"mov_to_addr", // mov_to_addr
		"jmp", // goto
		">=", // jge
		"<=", // jle
		"<", // jlt
		">", // jg
		"!=", // jne
		"==", // jeq
		"return", // op_return
		"push", // push_arg
		"unresolved call", // call_static
		"call", // plain_call
		"crazy_op" // crazy_op
};

jit_value useless_value = { Integer, Useless, 0 };

/**
 * Functions to deal with jit values
 */
jit_value jit_constant(int c)
{
	jit_value r = { Integer, Constant, 0 };
	r.value = c;
	return r;
}

jit_value jit_address(void* address)
{
	jit_value r = { ObjRef, Constant, 0 };
	r.value = (int32_t) address;
	return r;
}

jit_value jit_null()
{
	jit_value r = { ObjRef, Constant, 0 };
	r.value = 0;
	return r;
}

jit_value jit_local_field(int index, value_type type)
{
	jit_value r = { type, Local, 0 };
	r.value = (unsigned char) index;
	return r;
}

jit_value jit_label(int pos)
{
	jit_value r = { Integer, Label, 0 };
	r.value = pos;
	return r;
}

/**
 * Arithmetic operations
 */
jit_value Routine::jit_binary_operation(OP_QUAD op, jit_value op1,
                jit_value op2)
{
	Quadr r;
	if ((op1.meta.type == op2.meta.type)||
		(op1.meta.type == ArrRef && op2.meta.type == Integer) ||
		(op1.meta.type == ObjRef && op2.meta.type == Integer) ||
		(op1.meta.type == Byte && op2.meta.type == Integer)   ||
		(op1.meta.type == Integer && op2.meta.type == Byte)) {
		r.op1 = op1;
		r.op2 = op2;
		r.op = op;
		r.res.meta.type = Integer;//op2.meta.type;
		r.res.meta.scope = Temporal;
		r.res.value = getTempId();
		currentBB->q.push_back(r);
		if (op1.meta.scope == Temporal)
			freeTmp.insert(op1.value);
		if (op2.meta.scope == Temporal)
			freeTmp.insert(op2.value);
		return r.res;
	} else
		throw(std::runtime_error("Operating incompatible types"));
}

/**
 * Used to emit regular quadruplos
 */
jit_value Routine::jit_regular_operation(OP_QUAD op, jit_value op1,
                jit_value op2, value_type result_type)
{
	Quadr r;
	r.op1 = op1;
	r.op2 = op2;
	r.op = op;
	r.res.meta.type = result_type;
	r.res.meta.scope = Temporal;
	r.res.value = getTempId();
	currentBB->q.push_back(r);
	if (op1.meta.scope == Temporal)
		freeTmp.insert(op1.value);
	if (op2.meta.scope == Temporal)
		freeTmp.insert(op2.value);
	return r.res;
}

/**
 * Used to emit regular quadruplos
 */
void Routine::jit_regular_operation(OP_QUAD op, jit_value op1,
		jit_value op2, jit_value resultRef)
{
	Quadr r;
	r.op1 = op1;
	r.op2 = op2;
	r.op = op;
	r.res = resultRef;
	currentBB->q.push_back(r);
	if (op1.meta.scope == Temporal)
		freeTmp.insert(op1.value);
	if (op2.meta.scope == Temporal)
		freeTmp.insert(op2.value);
}

/**
 * Methods
 */
void Routine::jit_return_int(jit_value r)
{
	Quadr result = { OP_RETURN, r, useless_value, useless_value};
	currentBB->q.push_back(result);
}

void Routine::jit_return_void(void)
{
	Quadr result = { OP_RETURN, useless_value, useless_value, useless_value
	                };
	currentBB->q.push_back(result);
}

/**
 * assignaments
 */
void Routine::jit_assign_local(jit_value local, jit_value v)
{
	Quadr result = { ASSIGN, v, useless_value, local};
	if (v.meta.scope == Temporal)
		freeTmp.insert(v.value);
	currentBB->q.push_back(result);
}

jit_value jit::Routine::jit_copy(jit_value v)
{
	Quadr r;
	switch(v.meta.scope) {
	case Constant:
	case Useless:
	case Label:
		return v;
	case Temporal:
	case Local:
		// Fuck, I need to copy to a new location in a crazy way
		r.op1 = v;
		r.op2 = useless_value;
		r.op = CRAZY_OP;

		r.res.meta.type = v.meta.type;
		r.res.meta.scope = Temporal;
		r.res.value = getTempId();
		currentBB->q.push_back(r);
		return r.res;
	default:
		throw(runtime_error("Really? Is there some other kind of value"));
	}
}

/**
 * Utils
 */
int Routine::getTempId()
{
	if (freeTmp.empty())
		return last_temp++;
	else {
		std::set<int>::iterator it = freeTmp.begin();
		int id = *it;
		freeTmp.erase(id);
		return id;
	}
}


}
