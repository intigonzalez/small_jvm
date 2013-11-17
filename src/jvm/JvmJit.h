/*
 * JvmJit.h
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#ifndef JVMJIT_H_
#define JVMJIT_H_

#include "JvmExecuter.h"
#include "jit/Quadru.h"
#include "jit/CodeSectionMemoryManager.h"

#include <map>
#include <utility>
#include <string>

using namespace std;

namespace jvm {

class JvmJit: public jvm::JvmExecuter {
private:

	map< pair<string,string> , void* > compiledMethods;

	jit::CodeSectionMemoryManager codeSection;

	void* compile(ClassFile* cf, MethodInfo* method);
	jit::Routine toQuadruplus(ClassFile* cf, MethodInfo* method);

	jit::jit_value getConstant(ClassFile* cf, u2 index, CodeAttribute* caller);
public:
	JvmJit(ClassLoader* loader, Space* space);
	virtual ~JvmJit();
	virtual void initiateClass(ClassFile* cf);
	virtual void execute(ClassFile* cf, MethodInfo* method) {};

	template <class Function>
	void execute(ClassFile* cf, MethodInfo* method, Function fn){
		void* addr = compile(cf, method);
		fn(this, addr);
	}
};

} /* namespace jit */
#endif /* JVMJIT_H_ */
