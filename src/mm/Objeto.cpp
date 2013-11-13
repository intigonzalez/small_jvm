/*
 * Objeto.cpp
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#include "Objeto.h"
#include <cstring>

namespace MemoryManagement {

	static ObjectHandler* m_singleton = 0;

	ObjectHandler::ObjectHandler() {

	}

	ObjectHandler* ObjectHandler::instance() {
		if (m_singleton == 0)
			m_singleton = new ObjectHandler();
		return m_singleton;
	}

	MemoryManagement::Objeto ObjectHandler::newObject(Clase* clase) {
		Objeto obj = Space::instance()->newObject(clase);
		return obj;
	}

	void* ObjectHandler::getMemberAddress(MemoryManagement::Objeto object, std::string memberName) {
		Clase* type = (Clase*) (object->clase);
		dword size = type->sizeUntil(memberName);
		char* tmp = (char*) (object);
		void* r = &tmp[sizeof(Clase*) + sizeof(MemoryManagement::Objeto) + size];
		return r;
	}

	void* ObjectHandler::getMemberAddress(MemoryManagement::Objeto object, int memberIndex) {
		Clase* type = (Clase*) (object->clase);
		dword size = type->sizeUntil(memberIndex);
		char* tmp = (char*) (object);
		void* r = &tmp[sizeof(Clase*) + sizeof(MemoryManagement::Objeto) + size];
		return r;
	}

	void ObjectHandler::assign(MemoryManagement::Objeto* dst, MemoryManagement::Objeto src) {
		// roots must be controlled
		Space::instance()->includeRoot(dst);
		*dst = src;
	}

	void ObjectHandler::assignMemberValue(MemoryManagement::Objeto obj, std::string memberName,
			void* value) {
		char* addrDst = (char*) (getMemberAddress(obj, memberName));
		char* addrSrc = (char*) (value);
		Clase* type = (Clase*) (obj->clase);
		int size = type->memberSize(memberName);
		while (size) {
			*addrDst = *addrSrc;
			addrDst++;
			addrSrc++;
			size--;
		}
	}

	void ObjectHandler::getMemberValue(MemoryManagement::Objeto obj, std::string memberName,
			void* value) {
		char* addrSrc = (char*) (getMemberAddress(obj, memberName));
		char* addrDst = (char*) (value);
		Clase* type = (Clase*) (obj->clase);
		int size = type->memberSize(memberName);
		while (size) {
			*addrDst = *addrSrc;
			addrDst++;
			addrSrc++;
			size--;
		}
	}

	int ObjectHandler::getArrayLength(Objeto obj) {
		char* tmp = (char*) (obj);
		int* tmp2 = (int*) ((&tmp[sizeof(ArrayType*) + sizeof(void*)]));
		int length = (tmp2[0]);
		return length;
	}

	void ObjectHandler::assignArrayElement(Objeto obj, int index, void* value) {
		char* tmp = (char*) (obj);
		char* tmp2 = (char*) ((&tmp[sizeof(ArrayType*) + sizeof(void*) + sizeof(int)]));
		int n = ((ArrayType*) (obj->clase))->getBaseType()->size();
		memcpy(&tmp2[n * index], value, n);
	}

	void* ObjectHandler::getArrayBuffer(Objeto obj) {
		char* tmp = (char*) (obj);
		char* tmp2 = (char*) ((&tmp[sizeof(ArrayType*) + sizeof(void*) + sizeof(int)]));
		return tmp2;
	}

	void ObjectHandler::getArrayElement(Objeto obj, int index, void* buff) {
		char* tmp = (char*) (obj);
		char* tmp2 = (char*) ((&tmp[sizeof(ArrayType*) + sizeof(void*) + sizeof(int)]));
		int n = ((ArrayType*) (obj->clase))->getBaseType()->size();
		//int* i = (int*) (&tmp2[n * index]);
		memcpy(buff, &tmp2[n * index], n);
	}
/* namespace MemoryManagement */

}

