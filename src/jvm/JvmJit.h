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
#include  "../utilities/ThreadPool.h"

#include <map>
#include <utility>
#include <string>
#include <functional>

using namespace std;

namespace jvm {

class JvmJit: public jvm::JvmExecuter {
private:

	std::unique_ptr<ThreadPool> pool;

	jit::CodeSectionMemoryManager codeSection;

	void* compile(ClassFile* cf, MethodInfo* method);
public:
	JvmJit(ClassLoader* loader, Space* space);
	virtual ~JvmJit();
	virtual void initiateClass(ClassFile* cf);

	virtual void execute(ClassFile* cf, MethodInfo* method, std::function<void(JvmExecuter*, void* addr)> fn){
		void* addr = compile(cf, method);
		fn(this, addr);
	}
};

} /* namespace jit */
#endif /* JVMJIT_H_ */
