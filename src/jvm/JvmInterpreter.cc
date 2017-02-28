/*
 * JvmInterpreter.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include "JvmInterpreter.h"

#include <iostream>

#include "down_calls.h"
#include "../jvmclassfile/JVMSpecUtils.h"

#include <stdint.h>

using namespace std;

namespace jvm {

	JvmInterpreter::JvmInterpreter(ClassLoader* loader, Space* space) : JvmExecuter(loader, space), StackHandle() {
	}

	JvmInterpreter::~JvmInterpreter() {
	}

	void JvmInterpreter::initiateClass(ClassFile& cf) {
		// do something
	}

	void JvmInterpreter::createNewRawArray(int type) {
		int n = popI();
		Objeto obj = newRawArray((RawArrayTypes)type,n);
		push(obj);
	}

	void JvmInterpreter::execute(ClassFile& cf, MethodInfo& method, std::function<void(JvmExecuter*, void* addr)> fn) {
		RuntimeValue v;
		Objeto ref;
		// JavaDataType type;
		jint b;
		int32_t branch1;
		uint8_t branch2;
		int32_t i2;
		long long l;
//		AttributeInfo* ai = method->attributes[0];
		string method_name = cf.getUTF(method.name_index);
		CodeAttribute* code = method.code;
		if (code != 0) {
			int index = 0;
			while (index < code->code_length) {
				uint8_t opcode = (uint8_t) (code->code[index]);
				switch (opcode) {
					case aconst_null:
						v.ref = 0;
						push(v.ref);
						index++;
						break;
					case aload:
						b = (uint8_t)code->code[index+1];
						push(getLocalRef(b));
						index += 2;
						break;
					case aload_0:
					case aload_1:
					case aload_2:
					case aload_3:
						b = opcode - aload_0;
						push(getLocalRef(b));
						index++;
						break;
					case astore_0:
					case astore_1:
					case astore_2:
					case astore_3:
						b = opcode - astore_0;
						setLocal(b, popRef());
						index++;
						break;
					case astore:
						b = (uint8_t)code->code[index+1];
						setLocal(b, popRef());
						index += 2;
						break;
//					case iconst_0:
//					case iconst_1:
//					case iconst_2:
//					case iconst_3:
//					case iconst_4:
//					case iconst_5:
//						push(opcode - iconst_0);
//						index++;
//						break;
					case lconst_0:
					case lconst_1:
						l = opcode - lconst_0;
						push(l);
						index++;
						break;
//					case istore_0:
//					case istore_1:
//					case istore_2:
//					case istore_3:
//						b = opcode - istore_0;
//						setLocal(b, popI());
//						index++;
//						break;
//					case istore:
//						i2 = (int32_t) code->code[index + 1];
//						setLocal(i2, popI());
//						index += 2;
//						break;
					case iastore:
					case castore:
						v = top(); pop(); // value
						i2 = popI(); // index
						ref = popRef();
						ObjectHandler::instance()->assignArrayElement(ref, i2, &v);
						index++;
						break;
//					case iload_0:
//					case iload_1:
//					case iload_2:
//					case iload_3:
//						b = opcode - iload_0;
//						push(getLocalI(b));
//						index++;
//						break;
//					case iload:
//						i2 = (int32_t) code->code[index + 1];
//						push(getLocalI(i2));
//						index += 2;
//						break;
					case iaload:
						i2 = popI();
						ref = popRef();
						ObjectHandler::instance()->getArrayElement(ref, i2, &v);
						push(v.i);
						index++;
						break;
					case caload:
						i2 = popI();
						ref = popRef();
						v.l = 0;
						ObjectHandler::instance()->getArrayElement(ref, i2, &v);
						push(v, CHAR);
						index++;
						break;
//					case bipush:
//						push((int) code->code[index + 1]);
//						index += 2;
//						break;
//					case if_icmpge:
//						b = popI();
//						a = popI();
//						if (a >= b) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case if_icmpne:
//						b = popI();
//						a = popI();
//						if (a != b) {
//							branch1 =(char)code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case ifge:
//						a = popI();
//						if (a>=0) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case ifeq:
//						a = popI();
//						if (!a) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case ifne:
//						a = popI();
//						if (a) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case ifnull:
//						ref = popRef();
//						if (!ref) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case ifnonnull:
//						ref = popRef();
//						if (ref) {
//							branch1 = (char) code->code[index + 1];
//							branch2 = (uint8_t)code->code[index + 2];
//							index += (branch1 << 8) | branch2;
//						} else
//							index += 3;
//						break;
//					case iadd:
//						b = popI();
//						a = popI();
//						push(a + b);
//						index++;
//						break;
//					case isub:
//						b = popI();
//						a = popI();
//						push(a - b);
//						index++;
//						break;
//					case imul:
//						b = popI();
//						a = popI();
//						push(a * b);
//						index++;
//						break;
//					case idiv:
//						b = popI();
//						a = popI();
//						push(a / b);
//						index++;
//						break;
//					case iinc:
//						a = code->code[index + 2];
//						b = getLocalI((uint8_t) code->code[index + 1]);
//						setLocal((uint8_t) code->code[index + 1], b + a);
//						index += 3;
//						break;
//					case op_goto:
//						branch1 = (int32_t) code->code[index + 1];
//						branch2 = (uint8_t)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//						break;
					case invokestatic:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t) code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						generateStaticCall(cf, i2, code);
						index += 3;
						break;
					case invokespecial:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						invokeSpecial(cf, i2, code);
						index += 3;
						break;
					case invokevirtual: // FIXME like the other for now, this is wrong
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						invokeSpecial(cf, i2, code);
						index += 3;
						break;
					case op_return:
						//tStack = p2;
						return;
//					case ireturn:
//						//a = executionStack[--tStack];
//						//tStack = p2;
//						//executionStack[tStack++] = a;
//						return;
//						break;
					case op_new:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						createNewObject(cf, i2);
						index += 3;
						break;
					case op_newarray:
						i2 = code->code[index + 1];
						createNewRawArray(i2);
						index += 2;
						break;
//					case arraylength:
//						i2 = ObjectHandler::instance()->getArrayLength(popRef());
//						push(i2);
//						index++;
//						break;
//					case op_dup:
//						v = top();
//						type = topType();
//						push(v, type);
//						index++;
//						break;
					case putfield:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t) code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldAccess(cf, i2);
						index += 3;
						break;
					case putstatic:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldStaticAccess(cf, i2);
						index += 3;
						break;
					case getfield:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldAccess(cf, i2, false);
						index += 3;
						break;
					case getstatic:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldStaticAccess(cf, i2, false);
						index += 3;
						break;
//					case ldc:
//						i2 = (uint8_t)code->code[index + 1];
//						putConstantInStack(cf, i2, code);
//						index += 2;
//						break;
					case sipush:
						branch1 = (uint8_t) code->code[index + 1];
						branch2 = (uint8_t)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						push(i2);
						index += 3;
						break;
					default:
						cerr << "Unknown opcode " << (unsigned) (uint8_t) code->code[index]
								<< " at " << cf.getClassName() << ":" << method_name << ":" << index << endl;
						throw runtime_error("Unknown opcode");
						break;
				} // switch
			} // while
			//tStack = p2;
		}
	}

	void JvmInterpreter::fieldAccess(const ClassFile& cf, int16_t fieldRef, bool toWrite) {
		CONSTANT_Fieldref_info* ci = (CONSTANT_Fieldref_info*)(cf.info[fieldRef - 1].get());
		int16_t class_i = ci->class_index;
		int16_t name_type = ci->name_and_type_index;
		auto clase = (CONSTANT_Class_info*)(cf.info[class_i - 1].get());
		auto name = (CONSTANT_NameAndType_info*)(cf.info[name_type - 1].get());
		string class_name = cf.getUTF(clase->name_index);
		string field_name = cf.getUTF(name->name_index);
		string fdescription = cf.getUTF(name->descriptor_index);
		loadAndInit(class_name);
		if (toWrite) {
			RuntimeValue value = top();
			pop();
			Objeto ref = popRef();
			// now put field value
			ObjectHandler::instance()->assignMemberValue(ref, field_name, &value);
		} else {
			Objeto ref = popRef();
			RuntimeValue value;
			ObjectHandler::instance()->getMemberValue(ref, field_name, &value);
			switch (fdescription[0]) {
				case 'I':
					push(value.i);
					break;
				default:
					push(value.ref);
					break;
			}
		}

	}

	void JvmInterpreter::fieldStaticAccess(const ClassFile& cf, int16_t fieldRef, bool toWrite) {
		CONSTANT_Fieldref_info* ci = (CONSTANT_Fieldref_info*)(cf.info[fieldRef - 1].get());
		int16_t class_i = ci->class_index;
		int16_t name_type = ci->name_and_type_index;
		auto clase = (CONSTANT_Class_info*)(cf.info[class_i - 1].get());
		auto name = (CONSTANT_NameAndType_info*)(cf.info[name_type - 1].get());
		string class_name = cf.getUTF(clase->name_index);
		string field_name = cf.getUTF(name->name_index);
		string fdescription = cf.getUTF(name->descriptor_index);
		loadAndInit(class_name);
		if (toWrite) {
			RuntimeValue value = top();
			pop();
			int index = metaclasses[class_name];
			Objeto ref = classObjects[index];
			// now put field value
			ObjectHandler::instance()->assignMemberValue(ref, field_name, &value);
		} else {
			int index = metaclasses[class_name];
			Objeto ref = classObjects[index];
			RuntimeValue value;
			ObjectHandler::instance()->getMemberValue(ref, field_name, &value);
			switch (fdescription[0]) {
				case 'I':
					push(value.i);
					break;
				default:
					push(value.ref);
					break;
			}
		}

	}

	void JvmInterpreter::generateStaticCall(ClassFile& cf, int32_t methodReference,
			CodeAttribute* caller) {
		Constant_Info * cii = cf.info[methodReference- 1].get();
		CONSTANT_Methodref_info* ci = (CONSTANT_Methodref_info*)(cii);
		int16_t class_i = ci->class_index;
		int16_t name_type = ci->name_and_type_index;
		CONSTANT_Class_info* clase = (CONSTANT_Class_info*)(cf.info[class_i - 1].get());
		CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(cf.info[name_type - 1].get());
		string class_name = cf.getUTF(clase->name_index);
		string method_name = cf.getUTF(name->name_index);
		string method_description = cf.getUTF(name->descriptor_index);
		ClassFile& tmp = loader->getClass(class_name);
		int16_t method_index = tmp.getCompatibleMethodIndex(method_name.c_str(),
				method_description.c_str());
		auto method = tmp.methods[method_index];
		if (method.access_flags & ACC_NATIVE) {
			cerr << "Native method call." << endl << "Calling class : " << cf.getClassName() << endl << "Method Called : " << class_name << "." << method_name << endl;
			throw runtime_error("Native method class. Not implemented");
		}
		CodeAttribute* code = method.code;
		// parameters
		// Now the amazing call, jajaja
		newFrame(caller->max_locals);
		int count = JVMSpecUtils::countOfParameter(method_description);
		// copy parameters
		RuntimeValue v;
		for (int i = 0; i < count; i++) {
			v = top();
			JavaDataType type = topType();
			pop();
			setLocal(count - i - 1, type, v);
		}
		execute(tmp, method, [](JvmExecuter* exec, void * addr) { });
		exitFrame(caller->max_locals, code->max_locals);
	}

	void JvmInterpreter::invokeSpecial(ClassFile& cf, int32_t methodReference, CodeAttribute* caller) {
		// FIXME : lots of problems here, see jvm7 reference
		CONSTANT_Methodref_info* ci = (CONSTANT_Methodref_info*)(cf.info[methodReference - 1].get());
		int16_t class_i = ci->class_index;
		int16_t name_type = ci->name_and_type_index;
		auto clase = (CONSTANT_Class_info*)(cf.info[class_i - 1].get());
		auto name = (CONSTANT_NameAndType_info*)(cf.info[name_type - 1].get());
		string class_name = cf.getUTF(clase->name_index);
		string method_name = cf.getUTF(name->name_index);
		string method_description = cf.getUTF(name->descriptor_index);
		ClassFile& tmp = loadAndInit(class_name);
		int16_t method_index = tmp.getCompatibleMethodIndex(method_name.c_str(),
				method_description.c_str());
		auto method = tmp.methods[method_index];
//		AttributeInfo* ai = mi->attributes[0];
		CodeAttribute* code = method.code;
		// parameters
		// Now the amazing call, jajaja
		newFrame(caller->max_locals);
		int count = JVMSpecUtils::countOfParameter(method_description);
		// copy parameters
		RuntimeValue v;
		for (int i = 0; i < count; i++) {
			v = top();
			JavaDataType type = topType();
			pop();
			setLocal(count - i - 1, type, v);
		}
		execute(tmp, method, [](JvmExecuter* exec, void * addr) { });
		exitFrame(caller->max_locals, code->max_locals);
	}

	void JvmInterpreter::createNewObject(const ClassFile& cf, int32_t classRef) {
		CONSTANT_Class_info* ci = (CONSTANT_Class_info*)(cf.info[classRef - 1].get());
		string class_name = cf.getUTF(ci->name_index);
		ClassFile& tmp = loadAndInit(class_name);
		Clase* clase = (Clase*) (buildInMemoryClass(tmp));
		Objeto obj = ObjectHandler::instance()->newObject(clase);
		push(obj);
	}

} /* namespace jvm */
