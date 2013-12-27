#include <map>
#include <string>
#include <iostream>

#include "down_calls.h"
#include "JvmJit.h"
#include "../jvmclassfile/JVMSpecUtils.h"

std::map<std::string, Type*> rawTypes;

void initDownCalls()
{
	Type* t = new IntType();
	rawTypes[t->getName()] = t;
	t = new JavaCharType();
	rawTypes[t->getName()] = t;
	t = new LongType();
	rawTypes[t->getName()] = t;
}

Objeto newRawArray(RawArrayTypes type, int length)
{
	Type* base;
	std::string name = "[";
	switch (type) {
	case T_INT: // int
		name += "I";
		base = rawTypes["int"];
		break;
	case T_CHAR:
		name += "C";
		base = rawTypes["char"];
		break;
	default:
		std::cout << "Error in " << __FILE__ << ":" << __FUNCTION__
		                << ": " << type << "," << length << std::endl;
		throw new std::exception();
		break;
	}
	ArrayType* aType = new ArrayType(name, base);
	Objeto obj = Space::instance()->newArray(aType, length);
	return obj;
}

Objeto newObject(ClassFile* cf, int idx) {
	std::string class_name = JVMSpecUtils::getClassNameFromClassRef(cf, idx);
	// ensure that the class is loaded
	jvm::JvmJit::instance()->loadAndInit(class_name);
	Clase* clazz = jvm::JvmJit::instance()->getClassType(class_name);
	return Space::instance()->newObject(clazz);
}

void* getAddressForLoadedMethod(int id)
{
	return jvm::JvmJit::instance()->getAddrFromCompilationJobId(id);
}

void* highlevel_loadClassCompileMethodAndPath(void* job)
{
	return jvm::JvmJit::instance()->getAddrFromLoadingJob((jvm::LoadingAndCompile*)job);
}

void* getStaticFieldAddress(ClassFile* clazzFile, int idxField)
{
	std::string clazzName = JVMSpecUtils::
			getClassNameFromFieldRef(clazzFile, idxField);
	std::string fieldName = JVMSpecUtils::
			getFieldNameFromFieldRef(clazzFile, idxField);
//	std::string fieldDescription = JVMSpecUtils::
//				getFieldDescriptionFromFieldRef(clazzFile, idxField);
	jvm::JvmJit::instance()->loadAndInit(clazzName);

	return jvm::JvmJit::instance()->getStaticFieldAddress(clazzName, fieldName);
}

int getFieldDisplacement(ClassFile* clazzFile, int idxField)
{
	std::string clazzName = JVMSpecUtils::
			getClassNameFromFieldRef(clazzFile, idxField);
	std::string fieldName = JVMSpecUtils::
			getFieldNameFromFieldRef(clazzFile, idxField);
	Clase* clazz = jvm::JvmJit::instance()->getClassType(clazzName);
	if (clazz)
		return clazz->sizeUntil(fieldName) + BASE_OBJECT_SIZE;
	else throw runtime_error("Trying to access a field from some already unload class, "
			"that is impossible because NEW should be called first");
}
