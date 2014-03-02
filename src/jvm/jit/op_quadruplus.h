#ifndef __OP_QUAD__
#define __OP_QUAD__

namespace jit {

enum OP_QUAD {
	PLUS, SUB, MUL, DIV, REM, IINC, SHL, SHR,
	AND, OR, SAR,
	ASSIGN,MOV_FROM_ADDR, MOV_TO_ADDR,
	GOTO, JGE, JLE, JLT, JG, JNE, JEQ,
	OP_RETURN,
	PUSH_ARG, CALL_STATIC, PLAIN_CALL,
	CRAZY_OP // FIXME: My GOD, What is this?????????????? UGLYYYYYYYY,
};

extern const char* pretty_printing[];

}

#endif
