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
#include <mutex>

using namespace std;

namespace jvm {

struct CompilationJob {
	ClassFile& cf;
	MethodInfo& method;
	CompilationJob(ClassFile& clazz, MethodInfo& m) : cf(clazz), method(m) { }
};

struct LoadingAndCompile {
	ClassFile& callerClass; // class that trigger the initialization of the new class
	int methodRef;
	LoadingAndCompile(ClassFile& parent, int ref):
		callerClass{parent}, methodRef{ref} { }
};

class JvmJit: public jvm::JvmExecuter {
private:

	std::unique_ptr<ThreadPool> pool;
	jit::CodeSectionMemoryManager codeSection;

	std::atomic<int> idJobs;
	std::map<int, CompilationJob*> jobs;
	std::mutex mutex_jobs;

	void* compile(ClassFile& cf, MethodInfo& method);
	JvmJit(ClassLoader* loader, Space* space);
public:

	virtual ~JvmJit();
	virtual void initiateClass(ClassFile& cf);

	virtual void execute(ClassFile& cf, MethodInfo& method, std::function<void(JvmExecuter*, void* addr)> fn){
		void* addr = compile(cf, method);
		fn(this, addr);
	}

	int addCompilationJob(ClassFile& cf, MethodInfo& method);

	void* getAddrFromCompilationJobId(int id);
	void* getAddrFromLoadingJob(LoadingAndCompile* job);
	void* getStaticFieldAddress(std::string& class_name, std::string& fieldName);

	bool checkcast_impl(ClassFile& S, string& T);

	static JvmJit* instance();
};

} /* namespace jit */
#endif /* JVMJIT_H_ */
