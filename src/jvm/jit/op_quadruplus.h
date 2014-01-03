#ifndef __OP_QUAD__
#define __OP_QUAD__

namespace jit {

enum OP_QUAD {
	PLUS, SUB, MUL, DIV, REM, IINC, SHL, SHR = 7,
	ASSIGN,MOV_FROM_ADDR, MOV_TO_ADDR = 10,
	GET_ARRAY_POS, SET_ARRAY_POS = 12, //ARRAY_LEN,
	GOTO, JGE, JLE, JLT, JG, JNE, JEQ = 19,
	OP_RETURN = 20,
	PUSH_ARG, CALL_STATIC, PLAIN_CALL = 23,
	CRAZY_OP = 24, // FIXME: My GOD, What is this?????????????? UGLYYYYYYYY,
	AND, OR, SAR
};

}

#endif
