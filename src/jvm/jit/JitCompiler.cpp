/*
 * JitCompiler.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#include "JitCompiler.h"

#include "Simplex86Generator.h"
#include "../JvmJit.h"

#include "../../jvmclassfile/JVMSpecUtils.h"

#include <vector>
#include <stack>
#include <set>

using namespace std;

namespace jit {

JitCompiler::JitCompiler(jit::CodeSectionMemoryManager* section): codeSection(section) {

}

JitCompiler::~JitCompiler() {

}

void* JitCompiler::compile(ClassFile* cf, MethodInfo* method){
	// first generate IR with quadruplos
	Routine procedure = toQuadruplus(cf, method);
	// build control-flow graph
	procedure.buildControlFlowGraph();
	procedure.print();
	Simplex86Generator generator;
	void* addr = generator.generate(procedure, codeSection);
	return addr;
}

jit::Routine JitCompiler::toQuadruplus(ClassFile* cf, MethodInfo* method) {
	stack<jit_value> values;
	vector< pair< int, int > > bytecode2qua;
	set<int> labels;
	jit_value v1,v2,v;
//	Objeto ref;
	jint a, b;
	int32_t branch1;
	unsigned char branch2;
	int32_t i2;
	int count, count2;
	MethodInfo* tmp;


	std::string currentMethodDescriptor = cf->getUTF(method->descriptor_index);
	int argumentsCount = JVMSpecUtils::countOfParameter(currentMethodDescriptor);
	jit::Routine procedure(argumentsCount);

//	long long l;
	OP_QUAD oper;
	//AttributeInfo* ai = method->attributes[0];
	string method_name = cf->getUTF(method->name_index);
	CodeAttribute* code = method->code;
	if (!code) return procedure;
	int index = 0;
	while (index < code->code_length) {
		unsigned char opcode = (unsigned char) (code->code[index]);
		int nextIndex = procedure.q.size();
		bytecode2qua.push_back( pair<int, int>(index, nextIndex));
		switch (opcode) {
			case aconst_null:
				values.push(jit_null());
				index++;
				break;
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
			case astore_0:
			case astore_1:
			case astore_2:
			case astore_3:
				b = opcode - astore_0;
				procedure.jit_assign_local(jit_local_field(b, jit::ObjRef), values.top());
				values.pop();
//					setLocal(b, popRef());
				index++;
				break;
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
				i2 = (int32_t) code->code[index + 1];
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
				procedure.jit_regular_operation(SET_ARRAY_POS, v,v2, v1);
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
				i2 = (int32_t) code->code[index + 1];
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
				oper = JGE;
				if (opcode == if_icmple) oper = JLE;
				else if (opcode == if_icmpgt) oper = JG;
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
//						branch2 = (unsigned char)code->code[indeEsto es muy lindo 3 : 0xb7897008x + 2];
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
			case ifne:
				v1 = values.top(); values.pop();
				oper = JNE;
				branch1 = (char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				labels.insert(branch1);
				procedure.jit_regular_operation(oper, v1, useless_value, jit_label(branch1));
				index += 3;
				break;
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
				values.push(procedure.jit_regular_operation(GET_ARRAY_POS, v1,v2, Integer));
				index++;
				break;
			case iadd:
			case isub:
			case imul:
			case idiv:
			case irem:
				v2 = values.top(); values.pop();
				v1 = values.top(); values.pop();
				oper = PLUS;
				if (opcode == isub) oper = SUB;
				else if (opcode == imul) oper = MUL;
				else if (opcode == idiv) oper = DIV;
				else if (opcode == irem) oper = REM;
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
				procedure.jit_regular_operation(IINC, v1,v2, jit::useless_value);
				index += 3;
				break;
			case op_goto:
				branch1 = (char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				procedure.jit_regular_operation(GOTO, jit::useless_value,jit::useless_value, jit_label(branch1));
				labels.insert(branch1);
				index+=3;
				break;
			case invokestatic:
				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char) code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2);
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.top());
					values.pop();
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				tmp = getMethodToCall(cf, i2);
				if (tmp && tmp->address) {
					// the method is compiled
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					values.push(procedure.jit_regular_operation(
							PLAIN_CALL,
							jit_address(tmp->address),
							jit_constant(count2), Integer));
				} else {
					// TODO: the class is not loaded, generate stub method to:
					// 1 - Load the class
					// 2 - Fix the wrong pointer
					// 3 - Compile the method
					// 4 - Fix the wrong pointer
					// 5 - Call the method
					jvm::LoadingAndCompile* task =
							new jvm::LoadingAndCompile(cf, i2);
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					values.push(procedure.jit_regular_operation(
							CALL_STATIC,
							useless_value,
							jit_address(task),
							Integer));
				}
//					generateStaticCall(cf, i2, code);
//					values.push(jit_constant(123));
				index += 3;
				break;
			case invokespecial:
				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2) + 1; // +1 because of this
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.top());
					values.pop();
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				tmp = getMethodToCall(cf, i2);
				if (tmp && tmp->address) {
					// the method is compiled
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					procedure.jit_regular_operation(
							CALL_STATIC,
							jit_address(tmp->address),
							jit_constant(count2),
							Integer);
				} else {
					// TODO: the class is not loaded, generate stub method to:
					// 1 - Load the class
					// 2 - Fix the wrong pointer
					// 3 - Compile the method
					// 4 - Fix the wrong pointer
					// 5 - Call the method
					jvm::LoadingAndCompile* task =
							new jvm::LoadingAndCompile(cf, i2);
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					values.push(procedure.jit_regular_operation(
							CALL_STATIC,
							useless_value,
							jit_address(task),
							Integer));
				}
				index += 3;
				break;
			case invokevirtual:
				// FIXME I am doing something wrong because the implementation is the same
				// as invokespecial.
				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2) + 1; // +1 because of this
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.top());
					values.pop();
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				tmp = getMethodToCall(cf, i2);
				if (tmp && tmp->address) {
					// the method is compiled
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					values.push(procedure.jit_regular_operation(
							CALL_STATIC,
							jit_address(tmp->address),
							jit_constant(count2),
							Integer));
				} else {
					// TODO: the class is not loaded, generate stub method to:
					// 1 - Load the class
					// 2 - Fix the wrong pointer
					// 3 - Compile the method
					// 4 - Fix the wrong pointer
					// 5 - Call the method
					jvm::LoadingAndCompile* task =
							new jvm::LoadingAndCompile(cf, i2);
					// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
					values.push(procedure.jit_regular_operation(
							CALL_STATIC,
							useless_value,
							jit_address(task),
							Integer));
				}
				index += 3;
				break;
			case op_return:
				procedure.jit_return_void();
				index++;
				break;
			case ireturn:
				procedure.jit_return_int(values.top());
				values.pop();
				index++;
				break;
			case op_new:
				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;

				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG,
						jit_address(cf));

				values.push(procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&newObject),
						jit_constant(2),
						ObjRef));
				index += 3;
				break;
			case op_newarray:
				i2 = code->code[index + 1];
				v = values.top(); values.pop(); // size of the array

				procedure.jit_regular_operation(PUSH_ARG,
						v);

				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				values.push(procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&newRawArray),
						jit_constant(2),
						ArrRef));


//				values.push(procedure.jit_regular_operation(NEW_ARRAY, v, jit_constant(i2), ArrRef));
				//createNewRawArray(i2);
				index += 2;
				break;
			case arraylength:
				v = values.top(); values.pop();
//					i2 = ObjectHandler::instance()->getArrayLength(popRef());
				values.push(procedure.jit_regular_operation(ARRAY_LEN, v, jit::useless_value, Integer));
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
//			case putfield:
//				branch1 = (unsigned char) code->code[index + 1];
//				branch2 = (unsigned char) code->code[index + 2];
//				i2 = (branch1 << 8) | branch2;nextIndex
//				//
//				fieldAccess(cf, i2);
//				index += 3;
//				break;
			case putstatic:

				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;

				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG,
						jit_address(cf));

				v1 = procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&getStaticFieldAddress),
						jit_constant(2),
						Integer);
//
//				v1 = procedure.jit_regular_operation(
//						GET_STATIC_FIELD_ADDR,
//						jit_address(cf),
//						jit_constant(i2),
//						Integer);
				// FIXME, the same ugly assumption regarding member's type, Why Integer???
				v = values.top(); values.pop();
				procedure.jit_regular_operation(
						MOV_TO_ADDR,
						v1, v);
				index += 3;
				break;
//				case getfield:
//					branch1 = (unsigned char) code->code[index + 1];
//					branch2 = (unsigned char)code->code[index + 2];
//					i2 = (branch1 << 8) | branch2;
//					fieldAccess(cf, i2, false);
//					index += 3;
//					break;
			case getstatic:
				branch1 = (unsigned char) code->code[index + 1];
				branch2 = (unsigned char)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG,
						jit_address(cf));

				v1 = procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&getStaticFieldAddress),
						jit_constant(2),
						Integer);
//				v1 = procedure.jit_regular_operation(
//						GET_STATIC_FIELD_ADDR,
//						jit_address(cf),
//						jit_constant(i2),
//						Integer);
				// FIXME, the same ugly assumption regarding member's type, Why Integer???
				v2 = procedure.jit_regular_operation(
						MOV_FROM_ADDR,
						v1,
						useless_value,
						Integer);
				values.push(v2);
				index += 3;
				break;
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
		vector< pair< int, int > >::iterator ja = std::lower_bound(bytecode2qua.begin(), bytecode2qua.end(), pa,
				[] (pair<int, int>& pair, int v) {
					return pair.first < v;
				}
		);
		pair<int,int> p = *ja;
		procedure.q[p.second].label = p.first;
	}
	return procedure;
}

jit_value JitCompiler::getConstant(ClassFile* cf, int16_t index, CodeAttribute* caller) {
	Constant_Info* ri = (Constant_Info*)(cf->info[index - 1]);
	if (ri->tag() == CONSTANT_Integer) {
		CONSTANT_Integer_info* ii = (CONSTANT_Integer_info*)(ri);
		return jit_constant(ii->value);
	}
	else {
		CONSTANT_String_Info* si = (CONSTANT_String_Info*)(ri);
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
//					CodeAttribute* code = (CodeAttribute*>(ai);
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

MethodInfo* JitCompiler::getMethodToCall(ClassFile* cf, int16_t idx)
{
	std::string className = JVMSpecUtils::
			getClassNameFromMethodRef(cf, idx);

	ClassFile* otherClazz = jvm::JvmJit::instance()->getInitiatedClass(className);

	if (!otherClazz) return nullptr;

	std::string methodName = JVMSpecUtils::
			getMethodNameFromMethodRef(cf, idx);
	std::string methodDescription = JVMSpecUtils::
			getMethodDescriptionFromMethodRef(cf, idx);

	int16_t index = otherClazz->getCompatibleMethodIndex(methodName.c_str(),
					methodDescription.c_str());
	if (index >= 0 && index < otherClazz->methods_count) {
		return otherClazz->methods[index];
	}
	else throw new runtime_error("Trying to compile an non-existent method");
}

} /* namespace jit */
