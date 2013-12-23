
#include <map>
#include <string>
#include <iostream>

#include "down_calls.h"
#include "JvmJit.h"

std::map<std::string, Type*> rawTypes;

void initDownCalls() {
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
			std::cout << "Error in "<< __FILE__ << ":" << __FUNCTION__ << ": " << type << "," << length << std::endl;
			throw new std::exception();
			break;
	}
	ArrayType* aType = new ArrayType(name, base);
	Objeto obj = Space::instance()->newArray(aType, length);
	return obj;
}

void* getAddressForLoadedMethod(int id) {
	return jvm::JvmJit::instance()->getAddrFromCompilationJobId(id);
}
