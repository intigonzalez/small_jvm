/*
 * JvmExecuter.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include "JvmExecuter.h"
#include <exception>
#include <iostream>
#include <stack>
#include <dlfcn.h>

 #include "down_calls.h"

using namespace std;

namespace jvm {

	JvmExecuter::JvmExecuter(ClassLoader* loader, Space* space) {
		this->loader = loader;
		this->space = space;
		this->countOfClassObjects = 0;
		
		initDownCalls();

		// jni
		initJNI();

		void* h = dlopen("libcorejvm.so", RTLD_LAZY);
		loadedLibraries.push_back(h);
	}

	JvmExecuter::~JvmExecuter() {

	}

	ClassFile* JvmExecuter::loadAndInit(string class_name) {
		// this operation is not affected by the garbage collector
		ClassFile* cf = loader->getClass(class_name.c_str());
		if (cf) {
			ClassFile* tmp = cf;
			stack<ClassFile*> classesToInitialize;
			while (tmp) {
				classesToInitialize.push(tmp);
				// do this in a different thread or not?
				// FIXME: this operation is affected by the garbage collector
				// FIXMEW: it also is problematic because it involves reading classes from Disk
				buildInMemoryClass(tmp);
				// this operation is not affected by the garbage collector
				tmp = loader->getParentClass(tmp);
			}
			// now, initialize classes
			while (!classesToInitialize.empty()) {
				tmp = classesToInitialize.top(); classesToInitialize.pop();
				initiateClass(tmp);
			}
		}
		return cf;
	}

	Type* JvmExecuter::getType(string javaDescription) {
		ClassFile* tmp;
		Type* t;
		Type* baseType;
		switch (javaDescription[0]) {
			case 'I':
				t = rawTypes["int"]; // FIXME : Use Singleton
				break;
			case 'C':
				t = rawTypes["char"]; // FIXME
				break;
			case 'J':
				t = rawTypes["long"]; // FIXME
				break;
			case 'L':
				javaDescription = javaDescription.substr(1, javaDescription.size() - 2);
				tmp = loader->getClass(javaDescription.c_str());
				t = buildInMemoryClass(tmp);
				break;
			case '[':
				baseType = getType(javaDescription.substr(1));
				t = new ArrayType(javaDescription, baseType);
				break;
			default:
				cerr << javaDescription << " line : " << __LINE__ << " FILE: " << __FILE__ << endl;
				throw new std::exception();
		}
		return t;
	}

	Type* JvmExecuter::buildInMemoryClass(ClassFile* cf) {

		string cname = cf->getClassName();
		if (classes.find(cname) != classes.end())
			return classes[cname];
		Clase* a = new Clase(cname);
		Clase* classA = new Clase(cname);
		classes[cname] = a;
		for (int i = 0; i < cf->fields_count; ++i) {

			FieldInfo* fi = cf->fields[i];
			string name = cf->getUTF(fi->name_index);
			string desc = cf->getUTF(fi->descriptor_index);
			Type* t = getType(desc);
			if ((fi->access_flags & ACC_STATIC) != 0)
				classA->addMember(name, t);
			else
				a->addMember(name, t);
		} // for

		Objeto metaClass = Space::instance()->newObject(classA);
		metaclasses[cname] = countOfClassObjects;
		classObjects[countOfClassObjects++] = metaClass;
		Space::instance()->includeRoot(&classObjects[countOfClassObjects - 1]);
		return a;
	}

	void JvmExecuter::callStaticNativeMethod(string signature, Clase* clazz) {
		vector<void*>::iterator it = loadedLibraries.begin();
		void* f = 0;
		while (it != loadedLibraries.end()) {
			void * h = *it;
			f = dlsym(h, signature.c_str());
			if (f)
				break;
		}
		if (f) {
			// call function

		}

	}

}

 /* namespace jvm */
