#ifndef __OP_QUAD__
#define __OP_QUAD__

namespace jit {

enum OP_QUAD {
	PLUS, SUB, MUL, DIV, REM, IINC,
	ASSIGN,
	GET_ARRAY_POS, SET_ARRAY_POS, ARRAY_LEN, NEW_ARRAY,
	GOTO, JGE, JLE, JG,
	OP_RETURN,
	PUSH_ARG, CALL_STATIC
};

}

#endif
