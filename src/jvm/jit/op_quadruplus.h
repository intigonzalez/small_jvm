#ifndef __OP_QUAD__
#define __OP_QUAD__

namespace jit {

enum OP_QUAD {
	PLUS, SUB, MUL, DIV, REM, IINC,
	ASSIGN,
	GET_ARRAY_POS, SET_ARRAY_POS, ARRAY_LEN,
	GOTO, JGE, JLE, JG,
	OP_RETURN
};

}

#endif
