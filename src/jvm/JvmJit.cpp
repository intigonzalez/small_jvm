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
#include "../jvmclassfile/JVMSpecUtils.h"

using namespace jit;

namespace jvm {

static JvmJit* m_JvmJit = nullptr;

JvmJit::JvmJit(ClassLoader* loader, Space* space) :
		jvm::JvmExecuter(loader, space), codeSection(0x100000)
{
	std::unique_ptr<ThreadPool> tmp(new ThreadPool(1));
	pool = std::move(tmp);
}

JvmJit::~JvmJit()
{
}

void JvmJit::initiateClass(ClassFile* cf)
{
	// FIXME : Execute this in new Threads
	int16_t index = cf->getCompatibleMethodIndex("<clinit>", "()V");
	if (index >= 0 && index < cf->methods_count)
		JvmExecuter::execute(cf, "<clinit>", "()V", this,
		                [](JvmExecuter* exec, void * addr) {
			                void(*mm)() = (void(*)())addr;
			                mm();
		                });
}

void* JvmJit::compile(ClassFile* cf, MethodInfo* method)
{
	// fixme : Synchronize access to the method
	void* addr = nullptr;
	if (method->address)
		addr = method->address;
	else {
//		cout << "Compiling : " << cf->getClassName() << ":" << cf->getUTF(method->name_index) << '\n';
		// for now just one thread
		auto result =pool.get()->enqueue(
		                                [] (ClassFile* cf, MethodInfo* method, jit::CodeSectionMemoryManager* section) ->void* {
			                                jit::JitCompiler compiler(section);
			                                return compiler.compile(cf, method);
		                                }, cf, method, &codeSection);

		addr = result.get();
		method->address = addr;
		method->cleanCode();

		cout << "Method " << cf->getClassName() << ":"
			                << cf->getUTF(method->name_index) << " compiled"
			                << endl;
	}
	cout << "Method " << cf->getClassName() << ":"
	                << cf->getUTF(method->name_index) << " is in address : "
	                << addr << endl;
	return addr;
}

int JvmJit::addCompilationJob(ClassFile* cf, MethodInfo* method)
{
	std::unique_lock<std::mutex> lock(mutex_jobs);
	int id = idJobs++;
	jobs[id] = new CompilationJob(cf, method);
	return id;
}

void* JvmJit::getAddrFromCompilationJobId(int id)
{
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

void* JvmJit::getAddrFromLoadingJob(LoadingAndCompile* job)
{
	std::string className = JVMSpecUtils::
			getClassNameFromMethodRef(job->callerClass,
					job->methodRef);
	std::string methodName = JVMSpecUtils::
			getMethodNameFromMethodRef(job->callerClass,
					job->methodRef);
	std::string methodDescription = JVMSpecUtils::
			getMethodDescriptionFromMethodRef(job->callerClass,
					job->methodRef);

	// FIXME: find a way to remove job from memory,
	// remember that this is hard to do because many threads can execute the same code at the same time
	// and they can be using the reference while one thread is removing the reference
	// delete job

	ClassFile* calleeClazz = loadAndInit(className);
	int16_t index = calleeClazz->getCompatibleMethodIndex(methodName.c_str(),
					methodDescription.c_str());
	if (index >= 0 && index < calleeClazz->methods_count) {
		MethodInfo* m = calleeClazz->methods[index];
		return compile(calleeClazz, m);
	}
	throw new runtime_error("Trying to compile an non-existent method");
}



void* JvmJit::getStaticFieldAddress(std::string& class_name,
                std::string& fieldName)
{
	int index = metaclasses[class_name];
	Objeto ref = classObjects[index];
	return ObjectHandler::instance()->getMemberAddress(ref, fieldName);
}

JvmJit* JvmJit::instance()
{
	if (!m_JvmJit) {
		m_JvmJit = new JvmJit(ClassLoader::Instance(),
		                Space::instance());
	}
	return m_JvmJit;
}

} /* namespace jit */
