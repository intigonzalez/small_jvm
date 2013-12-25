/*
 * JvmExecuter.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef JVMEXECUTER_H_
#define JVMEXECUTER_H_

#include <map>
#include <vector>
#include <functional>

#include "../jvmclassfile/classfile.h"
#include "../jvmclassfile/classloader.h"

#include "../mm/Clase.h"
#include "../mm/Space.h"

#include "jni.h"

using namespace std;

using namespace MemoryManagement;

void initJNI();

namespace jvm {

	extern JNIEnv* env;

class JvmExecuter {
protected:
	ClassLoader* loader;
	Space* space;
	map<string, Type*> classes;

	vector<void*> loadedLibraries;
	map<string, void*> nativeMethods;

	int countOfClassObjects;
	Objeto classObjects[1000];
	map<string, int> metaclasses;

	virtual void initiateClass(ClassFile* cf) = 0;

public:
	JvmExecuter(ClassLoader* loader, Space* space);
	virtual ~JvmExecuter();

	/**
	 * Load a class following the JVM Specification.
	 * 1 - Load the class from its binary form and all its super-classes and interfaces
	 * 2 - Verify all the classes involved: FIXME: do it
	 * 3 - Start initial-default values for all the static fields
	 * 4 - Compile class initializers
	 * 5 - Execute class initializers
	 * 6 - Return the class-file ready to be used
	 */
	ClassFile* loadAndInit(string class_name);

	virtual void execute(ClassFile* cf, MethodInfo* method, std::function<void(JvmExecuter*, void* addr)> fn) = 0;

	static void execute(ClassFile* cf, const char* method, const char* description,
			JvmExecuter* exec, std::function<void(JvmExecuter*, void* addr)> fn) {
		int16_t index = cf->getCompatibleMethodIndex(method, description);
		if (index < 0 || index >= cf->methods_count)
			throw new exception();

		MethodInfo* mi = cf->methods[index];

		if ((mi->access_flags & ACC_STATIC))
			exec->execute(cf, mi, fn);
		else throw new exception(/*"Executing non-static method as static"*/);
	}

	static int countParameters(string s);

	Type* getType(string javaDescription);
	Type* buildInMemoryClass(ClassFile* cf); // fixme : This should be private and thread safe

	void callStaticNativeMethod(string signature, Clase* clazz);

	// nullptr is the class i not loaded
	ClassFile* getInitiatedClass(std::string& class_name);
};

} /* namespace jvm */
#endif /* JVMEXECUTER_H_ */
