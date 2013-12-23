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

static JvmJit* m_JvmJit = nullptr;

JvmJit::JvmJit(ClassLoader* loader, Space* space) : jvm::JvmExecuter(loader, space), codeSection(0x100000) {
	std::unique_ptr<ThreadPool> tmp(new ThreadPool(1));
	pool = std::move(tmp);
}

JvmJit::~JvmJit() { }

void JvmJit::initiateClass(ClassFile* cf) {

}

void* JvmJit::compile(ClassFile* cf, MethodInfo* method){
	// fixme : Synchronize access to the method
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

int JvmJit::addCompilationJob(ClassFile* cf, MethodInfo* method) {
	std::unique_lock<std::mutex> lock(mutex_jobs);
	int id = idJobs++;
	jobs[id] = new CompilationJob(cf, method);
	return id;
}

void* JvmJit::getAddrFromCompilationJobId(int id) {
	std::unique_lock<std::mutex> lock(mutex_jobs);
	if (jobs.find(id) != jobs.end()) {
		CompilationJob* job = jobs[id];
		// FIXME: This id is a shit by definition. We must rely on Classfile and MethodInfo to identify the target
		//jobs.erase(id);
		lock.unlock();
		ClassFile* cf = job->cf;
		MethodInfo* method = job->method;
		//delete job;
		return compile(cf, method);
	}
	throw std::runtime_error("Id for compilation job does not exist!!!");
}

JvmJit* JvmJit::instance() {
	if (!m_JvmJit) {
		m_JvmJit = new JvmJit(ClassLoader::Instance(), Space::instance());
	}
	return m_JvmJit;
}

} /* namespace jit */
