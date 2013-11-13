/*
 * Space.cpp
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#include "Space.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/times.h>
#include <cstring>

namespace MemoryManagement {

	static Space* m_Space = 0;

	Space* Space::instance() {
		if (m_Space == 0) {
			m_Space = new Space(1024 * 1024);
		}
		return m_Space;
	}

	Space::Space(long size) {
		address = 0;
		addressInmortal = 0;
		extraRoots = 0;
		setSpaceSize(size);
	}

	void Space::setSpaceSize(int size) {
		if (address) {
			delete[] address;
			delete[] addressInmortal;
		}
		halfspace = size >> 1;
		size = halfspace << 1;
		address = new char[size];
		addressInmortal = new char[size];
		mark0 = address;
		char* tmp = (char*) ((mark0));
		mark1 = &tmp[halfspace];
		firstFree = mark0;
		freeSpace = halfspace;
		firstInmortal = addressInmortal;
		freeInmortalSpace = size;
		_totalTime = 0;
	}

	Space::~Space() {
		delete[] address;
	}

	MemoryManagement::Objeto Space::fastAlloc(Clase* clase, int n) {
		MemoryManagement::Objeto r = (MemoryManagement::Objeto)(((void*) ((firstFree))));
		memset(firstFree, 0, n);
		firstFree += n;
		freeSpace -= n;
		r->clase = clase;
		r->forwardPointer = 0;
		return r;
	}

	MemoryManagement::Objeto Space::newObject(Clase* clase) {
		int n = (clase->_totalSize);
		n += sizeof(Clase*) + sizeof(void*);
		// reservations are done from mark0
		if (freeSpace < n) {
			printf("Collecting\n");
			doCollection();
			if (freeSpace < n) {
				printf("No enough memory\n");
				exit(0);
			}
			printf("End Collecting\n");
		}

		return fastAlloc(clase, n);
	}

	MemoryManagement::Objeto Space::newArray(ArrayType* a, int length) {
		int n = a->getBaseType()->size() * length + sizeof(int);
		n += sizeof(ArrayType*) + sizeof(void*);
		if (freeSpace < n) {
			printf("Collecting\n");
			doCollection();
			if (freeSpace < n) {
				printf("No enough memory\n");
				exit(0);
			}
			printf("End Collecting\n");
		}
		MemoryManagement::Objeto r = (MemoryManagement::Objeto)(((void*) ((firstFree))));
		memset(firstFree, 0, n);
		firstFree += n;
		freeSpace -= n;
		char* tmp = (char*)r;
		int* tmp2 = (int*)(&tmp[sizeof(ArrayType*) + sizeof(void*)]);
		tmp2[0] = length;

		r->clase = a;
		r->forwardPointer = 0;
		return r;
	}

	void Space::includeRoot(MemoryManagement::Objeto* obj) {
		//if (roots.find(obj) == roots.end()) {
		//	roots.insert(obj);
		//}
		roots.push_back(obj);
	}

	void Space::removeRoot(MemoryManagement::Objeto* obj) {
		for (std::vector<Objeto*>::iterator it = roots.begin(); it != roots.end(); it++) {
			Objeto* pointer = (Objeto*) ((*it));
			if (pointer == obj) {
				roots.erase(it);
				return;
			}
		}

	}

	void Space::removeRoots(int count) {
		while (count--)
			roots.pop_back();
	}

	void Space::copy(Objeto obj) {
		Type* t = (Type*)obj->clase;

		if (!t->isArray()) {
			Clase* clase = (Clase*)obj->clase;
			int n = clase->size() + sizeof(Clase*) + sizeof(Objeto);
			memcpy(firstFree, obj, n);
			obj->forwardPointer = firstFree;
			firstFree += n;
			freeSpace -= n;
		}
		else {
			ArrayType* arrayType = ((ArrayType*)obj->clase);
			if (arrayType) {
				char* tmp = (char*)obj;
				int* tmp2 = (int*)(&tmp[sizeof(ArrayType*) + sizeof(void*)]);
				int length = (tmp2[0]);
				int n = arrayType->getBaseType()->size() * length + sizeof(ArrayType*) + sizeof(void*) + sizeof(int);
				memcpy(firstFree, obj, n);
				obj->forwardPointer = firstFree;
				firstFree += n;
				freeSpace -= n;
			}
		}
	}

	void Space::doCollection() {
		clock_t a, d;
		tms myTime;
		times(&myTime);
		a = myTime.tms_utime;
		// swap spaces
		char* tmp = mark0;
		mark0 = mark1;
		mark1 = tmp;
		firstFree = mark0;
		freeSpace = halfspace;
		// Cheney iterative implementation
		// put every root on the queue
		char* scan = firstFree;
		for (std::vector<Objeto*>::iterator it = roots.begin(); it != roots.end(); it++) {
			Objeto* pointer = (Objeto*) ((*it));
			Objeto obj = pointer[0];
			if (obj) {
				if (!obj->forwardPointer)
					copy(obj);

				pointer[0] = (Objeto)((obj->forwardPointer));
			}
		}

		// now I start taking elements from the queue
		while (scan != firstFree) {
			Objeto obj = (Objeto)((scan));
			Type* t = (Type*)obj->clase;

			int n = 0;
			if (!t->isArray()) {
				Clase* clase = ((Clase*) (obj->clase));
				n = clase->size() + sizeof(Clase*) + sizeof(Objeto);
				for (int i = 0; i < clase->membersCount(); i++) {
					Member* m = clase->getMember(i);
					if (!m->getType()->IsRaw()) {
						// explore the field value
						Objeto* addr = (Objeto*) ((ObjectHandler::instance()->getMemberAddress(obj, i)));
						Objeto tmp = *addr;
						if (tmp) {
							if (!tmp->forwardPointer)
								copy(tmp);

							*addr = (Objeto)((tmp->forwardPointer));
						}
					}

				}
			}
			else {
				ArrayType* arrayType = ((ArrayType*)obj->clase);
				if (arrayType) {
					char* tmp = (char*)obj;
					int* tmp2 = (int*)(&tmp[sizeof(ArrayType*) + sizeof(void*)]);
					int length = (tmp2[0]);
					n = arrayType->getBaseType()->size() * length + sizeof(ArrayType*) + sizeof(void*) + sizeof(int);
					Objeto* data = (Objeto*)&tmp2[1];
					if (!arrayType->getBaseType()->IsRaw()) {
						for (int i = 0 ; i < length ; i++) {
							Objeto tmp = data[i];
							if (tmp) {
								if (!tmp->forwardPointer)
									copy(tmp);
								data[i] = (Objeto)((tmp->forwardPointer));
							}
						} // recorriendo los elementos del arreglo
					} // si el tipo base del arreglo no es raw
				} // si es un arreglo
			} // sno no es una clase

			scan += n;
		}

		times(&myTime);
		d = myTime.tms_utime;
		_totalTime += (d - a);
	}

	void* Space::explore(Objeto obj) {
		char buff[16];
		if (obj == NIL)
			return NIL;

		if (obj->forwardPointer != 0)
			return obj->forwardPointer;

		Clase* clase = (Clase*) ((obj->clase));
		int n = (clase->_totalSize);
		n += sizeof(Clase*) + sizeof(void*);
		Objeto newAddress = fastAlloc(clase, n);
		// set forward pointer
		obj->forwardPointer = newAddress;
		// copy the object to the new location, field by field
		for (int i = 0; i < clase->membersCount(); i++) {
			Member* m = clase->getMember(i);
			if (!m->getType()->IsRaw()) {
				// explore the field value
				Objeto* addr = (Objeto*) ((ObjectHandler::instance()->getMemberAddress(obj, i)));
				Objeto x = (Objeto)((explore(*addr)));
				addr = (Objeto*) ((ObjectHandler::instance()->getMemberAddress(newAddress, i)));
				*addr = x;
			} else {
				//int* addr = (int*)ObjectHandler::instance()->getMemberAddress(obj, i);
				//int value = *addr;
				//addr = (int*)ObjectHandler::instance()->getMemberAddress(newAddress, i);
				//*addr = value;
				ObjectHandler::instance()->getMemberValue(obj, m->getName(), buff);
				ObjectHandler::instance()->assignMemberValue(newAddress, m->getName(), buff);
			}
		}

		return newAddress;
	}



	void Space::setExtraRootsProvider(RootsProvider* provider) {
		extraRoots = provider;
	}
/* namespace MemoryManagement */

}

