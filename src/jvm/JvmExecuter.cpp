/*
 * JvmExecuter.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include "JvmExecuter.h"
#include <exception>
#include <iostream>
#include <dlfcn.h>

using namespace std;

namespace jvm {

	JvmExecuter::JvmExecuter(ClassLoader* loader, Space* space) {
		this->loader = loader;
		this->space = space;
		this->countOfClassObjects = 0;
		Type* t = new IntType();
		classes[t->getName()] = t;
		t = new JavaCharType();
		classes[t->getName()] = t;
		t = new LongType();
		classes[t->getName()] = t;



		// jni
		initJNI();

		void* h = dlopen("libcorejvm.so", RTLD_LAZY);
		loadedLibraries.push_back(h);
	}

	JvmExecuter::~JvmExecuter() {

	}

	void JvmExecuter::execute(ClassFile* cf, JvmExecuter* exec) {
		execute(cf, "main", "([Ljava/lang/String;)V", exec);
	}

	void JvmExecuter::execute(ClassFile* cf, const char* method, const char* description,
			JvmExecuter* exec) {
		u2 index = cf->getCompatibleMethodIndex(method, description);
		if (index < 0 || index >= cf->methods.size())
			throw new exception();

		MethodInfo* mi = cf->methods[index];

		if ((mi->access_flags & ACC_STATIC)) {
			exec->execute(cf, mi);
		} else
			throw new exception();
	}

	int JvmExecuter::execute_return_int(ClassFile* cf, const char* method, const char* description,
				JvmExecuter* exec) {
			u2 index = cf->getCompatibleMethodIndex(method, description);
			if (index < 0 || index >= cf->methods.size())
				throw new exception();

			MethodInfo* mi = cf->methods[index];

			if ((mi->access_flags & ACC_STATIC)) {
				return exec->execute_int(cf, mi);
			} else
				throw new exception();
		}




	int JvmExecuter::countParameters(string s) {
		if (s == "")
			return 0;
		long int index;
		switch (s[0]) {
			case '[':
				return countParameters(s.substr(1));
			case 'L':
				index = s.find(';');
				return 1 + countParameters(s.substr(index + 1));
			default:
				return 1 + countParameters(s.substr(1));
		}
	}

	ClassFile* JvmExecuter::loadAndInit(string class_name) {
		ClassFile* cf = loader->getClass(class_name.c_str());
		buildInMemoryClass(cf);
		initiateClass(cf);
		return cf;
	}

	Type* JvmExecuter::getType(string javaDescription) {
		ClassFile* tmp;
		Type* t;
		Type* baseType;
		switch (javaDescription[0]) {
			case 'I':
				t = classes["int"]; // FIXME : Use Singlenton
				break;
			case 'C':
				t = classes["char"]; // FIXME
				break;
			case 'J':
				t = classes["long"]; // FIXME
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
		for (int i = 0; i < cf->fields_count; i++) {

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
		u2 index = cf->getCompatibleMethodIndex("<clinit>", "()V");
		if (index >= 0 && index < cf->methods.size())
			JvmExecuter::execute(cf, "<clinit>", "()V", this);
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

	Objeto JvmExecuter::createNewRawArray(int type, int n) {
		Type* base;
		string name = "[";
		switch (type) {
			case 10: // int
				name += "I";
				base = classes["int"];
				break;
			case 5:
				name += "C";
				base = classes["char"];
				break;
			default:
				throw new std::exception();
				break;
		}
		ArrayType* aType = new ArrayType(name, base);
		Objeto obj = Space::instance()->newArray(aType, n);
		return obj;
	}

}

 /* namespace jvm */
