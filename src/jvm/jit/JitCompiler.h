/*
 * JitCompiler.h
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#ifndef JITCOMPILER_H_
#define JITCOMPILER_H_

#include "../JvmExecuter.h"
#include "Quadru.h"
#include "CodeSectionMemoryManager.h"

namespace jit {

class JitCompiler {
public:
	JitCompiler(jit::CodeSectionMemoryManager* section);
	virtual ~JitCompiler();

	void* compile(ClassFile* cf, MethodInfo* method);
private:
	jit::Routine toQuadruplus(ClassFile* cf, MethodInfo* method);
	jit::jit_value getConstant(ClassFile* cf, int16_t index, CodeAttribute* caller);

	MethodInfo* getMethodToCall(ClassFile* cf, int16_t idx);

	jit::CodeSectionMemoryManager* codeSection;
};

} /* namespace jit */

#endif /* JITCOMPILER_H_ */
