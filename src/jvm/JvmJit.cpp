/*
 * JvmJit.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: inti
 */

#include "JvmJit.h"
#include <stack>
#include <algorithm>
#include <set>
#include <vector>
#include "jit/Quadru.h"
#include "jit/Simplex86Generator.h"

using namespace jit;

namespace jvm {

bool myfunc(pair<int, int>& pair, int v) {
	return pair.first < v;
}

JvmJit::JvmJit(ClassLoader* loader, Space* space) : jvm::JvmExecuter(loader, space) { }

JvmJit::~JvmJit() { }

void JvmJit::initiateClass(ClassFile* cf) {

}

void JvmJit::execute(ClassFile* cf, MethodInfo* method){

}

jit::Routine JvmJit::toQuadruplus(ClassFile* cf, MethodInfo* method) {
	stack<jit_value> values;
	vector< pair< int, int > > bytecode2qua;
	set<int> labels;
	jit_value v1,v2,v;
//	Objeto ref;
	jint a, b;
	u4 branch1;
	unsigned char branch2;
	u4 i2;

	int argumentsCount = cf->getParameterCount(method->descriptor_index);
	jit::Routine procedure(argumentsCount);
//	long long l;
	int oper;
	AttributeInfo* ai = method->attributes[0];
	string method_name = cf->getUTF(method->name_index);
	CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);
	if (code != 0) {
		int index = 0;
		while (index < code->code_length) {
			unsigned char opcode = (unsigned char) (code->code[index]);
			int nextIndex = procedure.q.size();
			bytecode2qua.push_back( pair<int, int>(index, nextIndex));
			switch (opcode) {
//				case aconst_null:
//					v.ref = 0;
//					push(v.ref);
//					index++;
//					break;
				case aload:
					b = (unsigned char)code->code[index+1];
					values.push(jit_local_field(b, ObjRef));
//					push(getLocalRef(b));
					index += 2;
					break;
				case aload_0:
				case aload_1:
				case aload_2:
				case aload_3:
					b = opcode - aload_0;
					values.push(jit_local_field(b, ObjRef));
					index++;
//					push(getLocalRef(b));
					break;
//				case astore_0:
//				case astore_1:
//				case astore_2:
//				case astore_3:
//					b = opcode - astore_0;
//					setLocal(b, popRef());
//					index++;
//					break;
//				case astore:
//					b = (unsigned char)code->code[index+1];
//					setLocal(b, popRef());
//					index += 2;
//					break;
				case iconst_0:
				case iconst_1:
				case iconst_2:
				case iconst_3:
				case iconst_4:
				case iconst_5:
					values.push(jit_constant(opcode - iconst_0));
					index++;
					break;
//				case lconst_0:
//				case lconst_1:
//					l = opcode - lconst_0;
////					push(l);
//					index++;
//					break;
				case istore_0:
				case istore_1:
				case istore_2:
				case istore_3:
					b = opcode - istore_0;
					procedure.jit_assign_local(jit_local_field(b, jit::Integer), values.top());
					values.pop();
//					setLocal(b, popI());
					index++;
					break;
				case istore:
					i2 = (u4) code->code[index + 1];
					procedure.jit_assign_local(jit_local_field(i2, jit::Integer), values.top());
					values.pop();
//					setLocal(i2, popI());
					index += 2;
					break;
				case iastore:
//				case castore:
					v = values.top(); values.pop(); // r-value
					v2 = values.top(); values.pop(); // index
					v1 = values.top(); values.pop(); // array
					oper = ']';
					procedure.jit_regular_operation(oper, v,v2, v1);
					index++;
//					v = top(); pop(); // value
//					i2 = popI(); // index
//					ref = popRef();
//					ObjectHandler::instance()->assignArrayElement(ref, i2, &v);
					break;
				case iload_0:
				case iload_1:
				case iload_2:
				case iload_3:
					b = opcode - iload_0;
					values.push(jit_local_field(b, Integer));
//					push(getLocalI(b));
					index++;
					break;
				case iload:
					i2 = (u4) code->code[index + 1];
					values.push(jit_local_field(i2, Integer));
//					push(getLocalI(i2));
					index += 2;
					break;
//				case caload:
//					i2 = popI();
//					ref = popRef();
//					v.l = 0;
//					ObjectHandler::instance()->getArrayElement(ref, i2, &v);
//					push(v, CHAR);
//					index++;
//					break;
				case bipush:
					values.push(jit_constant((int) code->code[index + 1]));
//					push((int) code->code[index + 1]);
					index += 2;
					break;
				case if_icmpge:
				case if_icmple:
				case if_icmpgt:
					v2 = values.top(); values.pop(); // b
					v1 = values.top(); values.pop(); // a
					// jump if v1 >= v2
					branch1 = (char) code->code[index + 1];
					branch2 = (unsigned char)code->code[index + 2];
					branch1 = index + ((branch1 << 8) | branch2);
					labels.insert(branch1);
					oper = 3;
					if (opcode == if_icmple) oper = 4;
					else if (opcode == if_icmpgt) oper = 5;
					procedure.jit_regular_operation(oper, v1,v2, jit_label(branch1));
					index += 3;
					break;
//				case if_icmpne:
//					b = popI();
//					a = popI();
//					if (a != b) {
//						branch1 =(char)code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifge:
//					a = popI();
//					if (a>=0) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifeq:
//					a = popI();
//					if (!a) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifne:
//					a = popI();
//					if (a) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifnull:
//					ref = popRef();
//					if (!ref) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifnonnull:
//					ref = popRef();
//					if (ref) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (unsigned char)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
				case iaload:
					v2 = values.top(); values.pop();
					v1 = values.top(); values.pop();
					oper = '[';
					values.push(procedure.jit_regular_operation(oper, v1,v2, Integer));
					index++;
					break;
				case iadd:
				case isub:
				case imul:
				case idiv:
				case irem:
					v2 = values.top(); values.pop();
					v1 = values.top(); values.pop();
					oper = '+';
					if (opcode == isub) oper = '-';
					else if (opcode == imul) oper = '*';
					else if (opcode == idiv) oper = '/';
					else if (opcode == irem) oper = '%';
					values.push(procedure.jit_binary_operation(oper, v1,v2));
//					b = popI();
//					a = popI();
//					push(a + b);
					index++;
					break;
				case iinc:
					a = code->code[index + 2];
					v2 = jit_constant(a);
					b = (unsigned char) code->code[index + 1];
					v1 = jit_local_field(b, Integer);
//					setLocal((unsigned char) code->code[index + 1], b + a);
					procedure.jit_regular_operation(2, v1,v2, jit::useless_value);
					index += 3;
					break;
				case op_goto:
					branch1 = (char) code->code[index + 1];
					branch2 = (unsigned char)code->code[index + 2];
					branch1 = index + ((branch1 << 8) | branch2);
					procedure.jit_regular_operation(1, jit::useless_value,jit::useless_value, jit_label(branch1));
					labels.insert(branch1);
					index+=3;
					break;
//				case invokestatic:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char) code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					generateStaticCall(cf, i2, code);
//					index += 3;
//					break;
//				case invokespecial:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					invokeSpecial(cf, i2, code);
//					index += 3;
//					break;
//				case invokevirtual: // FIXME like the other for now, this is wrong
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					invokeSpecial(cf, i2, code);
//					index += 3;
//					break;
//				case op_return:
//					//tStack = p2;
//					return;
				case ireturn:
					//a = executionStack[--tStack];
					//tStack = p2;
					//executionStack[tStack++] = a;
					procedure.jit_return_int(values.top());
					values.pop();
					index++;
					break;
//				case op_new:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					createNewObject(cf, i2);
//					index += 3;
//					break;
//				case op_newarray:
//					i2 = code->code[index + 1];
//					createNewRawArray(i2);
//					index += 2;
//					break;
				case arraylength:
					v = values.top(); values.pop();
//					i2 = ObjectHandler::instance()->getArrayLength(popRef());
					values.push(procedure.jit_regular_operation('L', v, jit::useless_value, Integer));
//					push(i2);
					index++;
					break;
				case op_dup:
					v = values.top();
					values.push(v);
//					type = topType();
//					push(v, type);
					index++;
					break;
				case op_dup2:
					// FIXME : LONG and Double, see jvm specification
					v1 = values.top(); values.pop();
					v2 = values.top();
					values.push(v1);
					values.push(v2);
					values.push(v1);
					index++;
					break;
//				case putfield:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char) code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;nextIndex
//					fieldAccess(cf, i2);
//					index += 3;
//					break;
//				case putstatic:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					fieldStaticAccess(cf, i2);
//					index += 3;
//					break;
//				case getfield:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					fieldAccess(cf, i2, false);
//					index += 3;
//					break;
//				case getstatic:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					fieldStaticAccess(cf, i2, false);
//					index += 3;
//					break;
				case ldc:
					i2 = (unsigned char)code->code[index + 1];
					values.push(getConstant(cf, i2, code));
					index += 2;
					break;
				case sipush:
					branch1 = (unsigned char) code->code[index + 1];
					branch2 = (unsigned char)code->code[index + 2];
					i2 = (branch1 << 8) | branch2;
					values.push(jit_constant(i2));
					index += 3;
					break;
				default:
					cerr << "Unknown opcode " << (unsigned) (unsigned char) code->code[index]
							<< " at " << cf->getClassName() << ":" << method_name << ":" << index << endl;
					throw new exception();
					break;
			} // switch
		} // while
		for (auto& pa : labels){
			vector< pair< int, int > >::iterator ja = std::lower_bound(bytecode2qua.begin(), bytecode2qua.end(), pa, myfunc);
			pair<int,int> p = *ja;
			procedure.q[p.second].label = p.first;
		}

	}
	return procedure;
}

void* JvmJit::compile(ClassFile* cf, MethodInfo* method){
	void* addr = 0;
	if (method->address != 0) {
		addr = method->address;
	}
	else {
		cout << "Compiling : " << cf->getClassName() << ":" << cf->getUTF(method->name_index) << '\n';
		// first generate IR with quadruplos
		Routine procedure = toQuadruplus(cf, method);
		// build control-flow graph
		procedure.buildControlFlowGraph();
		procedure.print();
		Simplex86Generator generator;
		addr = generator.generate(procedure);
		method->address = addr;
	}
	return addr;
}

jit_value JvmJit::getConstant(ClassFile* cf, u2 index, CodeAttribute* caller) {
	Constant_Info* ri = dynamic_cast<Constant_Info*>(cf->info[index - 1]);
	CONSTANT_Integer_info* ii = dynamic_cast<CONSTANT_Integer_info*>(ri);
	if (ii)
		return jit_constant(ii->value);
	else {
		CONSTANT_String_Info* si = dynamic_cast<CONSTANT_String_Info*>(ri);
		if (si) {
//					string utf = cf->getUTF(si->index);
//					cout << utf << endl;
//					ArrayType* t = new ArrayType("",classes["char"]);
//					Objeto obj = Space::instance()->newArray(t, 0); // 0 length char array FIXME
//					Space::instance()->includeRoot(&obj);
//
//
//					ClassFile* cf2 = loadAndInit("java/lang/String");
//					Clase* clase = (Clase*) (buildInMemoryClass(cf2));
//					Objeto obj2 = ObjectHandler::instance()->newObject(clase);
//					push(obj2);
//
//					u2 i2 = cf2->getCompatibleMethodIndex("<init>","([C)V");
//					MethodInfo* mi = cf2->methods[i2];
//					AttributeInfo* ai = mi->attributes[0];
//					CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);
//
//					newFrame(caller->max_locals);
//					setLocal(0, obj2);
//					setLocal(1, obj);
//					execute(cf2, mi);
//					exitFrame(caller->max_locals, code->max_locals);
//					Space::instance()->removeRoot(&obj);
			return jit::useless_value;
		}
		else {
			return jit::useless_value;
//					push(0.0f);
		}
	}
}

} /* namespace jit */
