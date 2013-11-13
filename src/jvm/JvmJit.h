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

#include <map>
#include <utility>
#include <string>

using namespace std;

namespace jvm {

class JvmJit: public jvm::JvmExecuter {
private:

	map< pair<string,string> , void* > compiledMethods;

	void* compile(ClassFile* cf, MethodInfo* method);
	void toQuadruplus(ClassFile* cf, MethodInfo* method, jit::Routine& q);

	jit::jit_value getConstant(ClassFile* cf, u2 index, CodeAttribute* caller);
public:
	JvmJit(ClassLoader* loader, Space* space);
	virtual ~JvmJit();
	virtual void initiateClass(ClassFile* cf);
	virtual void execute(ClassFile* cf, MethodInfo* method);
	virtual int execute_int(ClassFile* cf, MethodInfo* method);
};

} /* namespace jit */
#endif /* JVMJIT_H_ */
