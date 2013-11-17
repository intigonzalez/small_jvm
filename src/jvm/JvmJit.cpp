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

bool myfunc(pair<int, int>& pair, int v) {
	return pair.first < v;
}

JvmJit::JvmJit(ClassLoader* loader, Space* space) : jvm::JvmExecuter(loader, space), codeSection(0x100000) {
	std::unique_ptr<ThreadPool> tmp(new ThreadPool(4));
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
		cout << "Compiling : " << cf->getClassName() << ":" << cf->getUTF(method->name_index) << '\n';

		// for now just one thread
		auto result = pool.get()->enqueue([] (ClassFile* cf, MethodInfo* method, jit::CodeSectionMemoryManager& section) ->void*{
			jit::JitCompiler compiler(section);
			return compiler.compile(cf, method);
		}, cf, method, codeSection);

		addr = result.get();
	}
	return addr;
}

} /* namespace jit */
