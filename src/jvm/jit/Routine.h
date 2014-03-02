#ifndef __ROUTINE_JAVA__
#define __ROUTINE_JAVA__

#include "../CFG.h"

namespace jit {

struct Routine {
	unsigned countOfParameters;
	jvm::CFG cfg;
	int last_temp;
	std::set<int> freeTmp;
	BasicBlock* currentBB;

	Routine(unsigned nbParameters) : countOfParameters(nbParameters), last_temp(0), currentBB(nullptr){ }

	Routine(Routine&& r) {
		countOfParameters = r.countOfParameters;
		last_temp = r.last_temp;
		freeTmp = r.freeTmp; r.freeTmp = {};
		currentBB = r.currentBB;
		cfg = r.cfg; r.cfg = {};
	}

	Routine() : countOfParameters(0), last_temp(0), currentBB(nullptr){ }

	/**
	 * Arithmetic operations
	 */
	jit_value jit_binary_operation(OP_QUAD op, jit_value op1, jit_value op2);

	/**
	 * Used to emit regular quadruplos
	 */
	jit_value jit_regular_operation(OP_QUAD op, jit_value op1, jit_value op2, value_type result_type);
	void jit_regular_operation(OP_QUAD op, jit_value op1, jit_value op2 =
	                useless_value, jit_value resultRef = useless_value);

	/**
	 * Methods
	 */
	void jit_return_int(jit_value r);
	void jit_return_void(void);

	/**
	 * Assignments
	 */
	void jit_assign_local(jit_value local, jit_value v);
	jit_value jit_copy(jit_value v);

	/**
	 * Utils
	 */
	int getTempId();
};

}

#endif
