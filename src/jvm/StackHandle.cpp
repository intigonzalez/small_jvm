/*
 * StackHandle.cpp
 *
 *  Created on: Sep 10, 2012
 *      Author: inti
 */

#include "StackHandle.h"

#include "../mm/Space.h"

using namespace std;
using namespace MemoryManagement;

namespace jvm {

	StackHandle::StackHandle() {
		tLocals = 0;
		t = 0;
	}

	bool StackHandle::hasNext() {
		return false;
	}

	Objeto* StackHandle::next() {
		return 0;
	}

	StackHandle::~StackHandle() {

	}

	void StackHandle::push(Objeto e) {
		RuntimeValue a;
		a.ref = e;
		_stack[t] = a;
		typesS[t++] = OBJETO;
		if (pos2.find(t - 1) == pos2.end()) {
			pos2.insert(t - 1);
			Space::instance()->includeRoot(&_stack[t - 1].ref);
		}
	}

	void StackHandle::push(RuntimeValue e, JavaDataType type) {
		_stack[t] = e;
		typesS[t++] = type;
		if (type == OBJETO && pos2.find(t - 1) == pos2.end()) {
			pos2.insert(t - 1);
			Space::instance()->includeRoot(&_stack[t - 1].ref);
		}
	}

	Objeto StackHandle::popRef() {
		RuntimeValue a = _stack[--t];
		if (pos2.find(t) != pos2.end()) {
			pos2.erase(t);
			Space::instance()->removeRoot(&_stack[t].ref);
		}
		return a.ref;
	}

	void StackHandle::pop() {
		t--;
		if (pos2.find(t) != pos2.end()) {
			pos2.erase(t);
			Space::instance()->removeRoot(&_stack[t].ref);
		}
	}

	void StackHandle::setLocal(int i, Objeto e) {
		locals[tLocals + i].ref = e;
		typesL[tLocals + i] = OBJETO;
		if (pos.find(tLocals + i) == pos.end()) {
			pos.insert(tLocals + i);
			Space::instance()->includeRoot(&locals[tLocals + i].ref);
		}
	}

	void StackHandle::setLocal(int i, JavaDataType type, RuntimeValue e) {
		locals[tLocals + i] = e;
		typesL[tLocals + i] = type;
		if (type == OBJETO && pos.find(tLocals + i) == pos.end()) {
			pos.insert(tLocals + i);
			Space::instance()->includeRoot(&locals[tLocals + i].ref);
		}
	}

	void StackHandle::exitFrame(int countOfLocalsInParent, int countOfLocalsInCurrent) {
		for (int k = 0; k < countOfLocalsInCurrent; k++) {
			if (pos.find(k + tLocals) != pos.end()) {
				pos.erase(tLocals + k);
				Space::instance()->removeRoot(&locals[tLocals + k].ref);
			}
		}
		tLocals -= countOfLocalsInParent;
	}

} /* namespace jvm */
