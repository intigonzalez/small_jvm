/*
 * JvmJit.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#include "JvmJit.h"
#include <stack>
#include <algorithm>
#include <set>
#include <vector>
#include "jit/Quadru.h"
#include "jit/Simplex86Generator.h"
#include "jit/JitCompiler.h"

using namespace jit;

namespace jvm {

JvmJit::JvmJit(ClassLoader* loader, Space* space) : jvm::JvmExecuter(loader, space), codeSection(0x100000) {
	std::unique_ptr<ThreadPool> tmp(new ThreadPool(1));
	pool = std::move(tmp);
}

JvmJit::~JvmJit() { }

void JvmJit::initiateClass(ClassFile* cf) {

}

void* JvmJit::compile(ClassFile* cf, MethodInfo* method){
	void* addr = nullptr;
	if (method->address)
		addr = method->address;
	else {
//		cout << "Compiling : " << cf->getClassName() << ":" << cf->getUTF(method->name_index) << '\n';

		// for now just one thread
		auto result = pool.get()->enqueue([] (ClassFile* cf, MethodInfo* method, jit::CodeSectionMemoryManager* section) ->void*{
			jit::JitCompiler compiler(section);
			return compiler.compile(cf, method);
		}, cf, method, &codeSection);

		addr = result.get();
		method->address = addr;
		method->cleanCode();
	}
	cout << "Method " << cf->getClassName() << ":" << cf->getUTF(method->name_index) << " is in address : " << addr << endl;
	return addr;
}

} /* namespace jit */
