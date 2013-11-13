/*
 * StackHandle.h
 *
 *  Created on: Sep 10, 2012
 *      Author: inti
 */

#ifndef STACKHANDLE_H_
#define STACKHANDLE_H_


#include "../mm/common.h"
#include "../mm/Space.h"
#include <stack>
#include <set>

#include "jni.h"

using namespace std;
using namespace MemoryManagement;

enum JavaDataType {INT, FLOAT, BOOL, DOUBLE, LONG, CHAR, OBJETO};

namespace jvm {

	union RuntimeValue {
				jint i;
				jfloat f;
				jboolean b;
				jdouble d;
				jlong l;
				jchar c;
				Objeto ref;
		};



	class StackHandle : public RootsProvider {
		private:
			RuntimeValue locals[1000];
			JavaDataType typesL[1000];
			int tLocals;
			int t;
			RuntimeValue _stack[1000];
			JavaDataType typesS[1000];

			set<int> pos;
			set<int> pos2;

		public:

			virtual bool hasNext();
			virtual Objeto* next();

			void push(jint e) { RuntimeValue a; a.i = e; _stack[t] = a; typesS[t++] = INT; }
			void push(jfloat e) { RuntimeValue a; a.f = e; _stack[t] = a; typesS[t++] = FLOAT; }
			void push(jboolean e) { RuntimeValue a; a.b = e; _stack[t] = a; typesS[t++] = BOOL; }
			void push(jdouble e) { RuntimeValue a; a.d = e; _stack[t] = a; typesS[t++] = DOUBLE; }
			void push(jlong e) { RuntimeValue a; a.l = e; _stack[t] = a; typesS[t++] = LONG; }
			void push(jchar e) { RuntimeValue a; a.c = e; _stack[t] = a; typesS[t++] = CHAR; }
			void push(Objeto e);
			void push(RuntimeValue e, JavaDataType type);

			jint popI() { RuntimeValue a = _stack[--t]; return a.i;	}
			jfloat popF() { RuntimeValue a = _stack[--t]; return a.f;	}
			jboolean popB() { RuntimeValue a = _stack[--t]; return a.b;	}
			jdouble popD() { RuntimeValue a = _stack[--t]; return a.d;	}
			jlong popL() { RuntimeValue a = _stack[--t]; return a.l;	}
			jchar popC() { RuntimeValue a = _stack[--t]; return a.c;	}
			Objeto popRef();
			RuntimeValue top() { RuntimeValue a = _stack[t-1]; return a; }
			JavaDataType topType() {	return typesS[t - 1];	}
			void pop();

			void newFrame(int countOfLocalsInCurrent) { tLocals += countOfLocalsInCurrent; }
			void exitFrame(int countOfLocalsInParent, int countOfLocalsInCurrent);

			void setLocal(int i, jint e) { locals[tLocals + i].i = e; typesL[tLocals + i] = INT;}
			void setLocal(int i, jfloat e) { locals[tLocals + i].f = e; typesL[tLocals + i] = FLOAT;}
			void setLocal(int i, jboolean e) { locals[tLocals + i].b = e; typesL[tLocals + i] = BOOL;}
			void setLocal(int i, jdouble e) { locals[tLocals + i].d = e; typesL[tLocals + i] = DOUBLE;}
			void setLocal(int i, jlong e) { locals[tLocals + i].l = e; typesL[tLocals + i] = LONG;}
			void setLocal(int i, jchar e) { locals[tLocals + i].c = e; typesL[tLocals + i] = CHAR;}
			void setLocal(int i, Objeto e);
			void setLocal(int i, JavaDataType type, RuntimeValue e);

			jint getLocalI(int i) { return locals[tLocals + i].i; }
			jfloat getLocalF(int i) { return locals[tLocals + i].f; }
			jboolean getLocalB(int i) { return locals[tLocals + i].b; }
			jdouble getLocalD(int i) { return locals[tLocals + i].d; }
			jlong getLocalL(int i) { return locals[tLocals + i].l; }
			jchar getLocalC(int i) { return locals[tLocals + i].c; }
			Objeto getLocalRef(int i) { return locals[tLocals + i].ref; }

			StackHandle();
			virtual ~StackHandle();
	};

} /* namespace jvm */
#endif /* STACKHANDLE_H_ */
