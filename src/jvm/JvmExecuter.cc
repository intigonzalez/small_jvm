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
#include "../utilities/Logger.h"

using namespace std;

namespace jvm {

JvmExecuter::JvmExecuter(ClassLoader* loader, Space* space)
{
	this->loader = loader;
	this->space = space;
	this->countOfClassObjects = 0;

	initDownCalls();

	// jni
	initJNI();

	void* h = dlopen("libcorejvm.so", RTLD_LAZY);
	loadedLibraries.push_back(h);
}

JvmExecuter::~JvmExecuter()
{

}

ClassFile& JvmExecuter::loadAndInit(const string& class_name)
{
	// this operation is not affected by the garbage collector
	LOG_DBG("Loading class: " + class_name);
	ClassFile& cf = loader->getClass(class_name);
	LOG_DBG("Class loaded: ", cf.getClassName(), " ", cf.info.size());
	if (classes.find(class_name) != classes.end()) {
		return cf;
	}
	ClassFile& tmp = cf;
	std::stack<std::string> classesToInitialize;
	while (!tmp.isObjectClass()) {
		classesToInitialize.push(tmp.getClassName());
		// do this in a different thread or not?
		// FIXME: this operation is affected by the garbage collector
		// FIXME: it also is problematic because it involves reading classes from Disk
		buildInMemoryClass(tmp);
		// this operation is not affected by the garbage collector
		LOG_DBG("jejej 111: " + tmp.getClassName());
		tmp = loader->getParentClass(tmp);
		LOG_DBG("jejej 222: " + class_name);
	}
	LOG_DBG("jejej: " + class_name);
	// now, initialize classes
	while (!classesToInitialize.empty()) {
		std::string name = classesToInitialize.top();
		classesToInitialize.pop();
		ClassFile& tmp = loader->getClass(name);
		initiateClass(tmp);
	}
	return cf;
}

ClassFile& JvmExecuter::getInitiatedClass(const std::string& class_name)
{
	// FIXME : Synchronize
	if (classes.find(class_name) != classes.end()) {
		return loader->getClass(class_name);
	}
	throw std::runtime_error("Unknown class: " + class_name);
}

Clase* JvmExecuter::getClassType(const std::string& class_name)
{
	// FIXME : Synchronize
	if (classes.find(class_name) != classes.end()) {
		// fixme: ensure that the type is a Class and not something else
		return (Clase*)classes[class_name];
	}
	return nullptr;
}

Type* JvmExecuter::getType(const std::string javaDescription)
{
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
	case 'F':
		t = rawTypes["float"]; // FIXME
		break;
	case 'Z':
		t = rawTypes["bool"]; // FIXME
		break;
	case 'L':
		{
			std::string rest = javaDescription.substr(1, javaDescription.size() - 2);
			ClassFile& tmp = loader->getClass(rest);
			t = buildInMemoryClass(tmp);
		}
		break;
	case '[':
		baseType = getType(javaDescription.substr(1));
		t = new ArrayType(javaDescription, baseType);
		break;
	default:
		cerr << javaDescription << " line : " << __LINE__ << " FILE: "
		                << __FILE__ << endl;
		throw std::runtime_error("Unknown Java type: " + javaDescription);
	}
	return t;
}

Type* JvmExecuter::buildInMemoryClass(const ClassFile& cf)
{
	string cname = cf.getClassName();
	if (classes.find(cname) != classes.end())
		return classes[cname];
	Clase* a = new Clase(cname);
	Clase* classA = new Clase(cname);
	classes[cname] = a;
	for (const auto& fi: cf.fields) {
		string name = cf.getUTF(fi.name_index);
		string desc = cf.getUTF(fi.descriptor_index);
		Type* t = getType(desc);
		if ((fi.access_flags & ACC_STATIC) != 0)
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

void JvmExecuter::callStaticNativeMethod(string signature, Clase* clazz)
{
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
