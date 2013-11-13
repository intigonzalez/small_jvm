/*
 * JvmInterpreter.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include "JvmInterpreter.h"

#include <iostream>

using namespace std;

namespace jvm {

	JvmInterpreter::JvmInterpreter(ClassLoader* loader, Space* space) : JvmExecuter(loader, space), StackHandle() {
	}

	JvmInterpreter::~JvmInterpreter() {
	}

	void JvmInterpreter::initiateClass(ClassFile* cf) {
		// do something
	}

	void JvmInterpreter::createNewRawArray(int type) {
		int n = popI();
		Objeto obj = JvmExecuter::createNewRawArray(type, n);
		push(obj);
	}

	int JvmInterpreter::execute_int(ClassFile* cf, MethodInfo* method) {
		execute(cf, method);
		return this->popI();
	}

	void JvmInterpreter::execute(ClassFile* cf, MethodInfo* method) {
		RuntimeValue v;
		Objeto ref;
		JavaDataType type;
		jint a, b;
		u4 branch1;
		unsigned char branch2;
		u4 i2;
		long long l;
		AttributeInfo* ai = method->attributes[0];
		string method_name = cf->getUTF(method->name_index);
		CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);
		if (code != 0) {
			int index = 0;
			while (index < code->code_length) {
				unsigned char opcode = (unsigned char) (code->code[index]);
				switch (opcode) {
					case aconst_null:
						v.ref = 0;
						push(v.ref);
						index++;
						break;
					case aload:
						b = (unsigned char)code->code[index+1];
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
						b = (unsigned char)code->code[index+1];
						setLocal(b, popRef());
						index += 2;
						break;
					case iconst_0:
					case iconst_1:
					case iconst_2:
					case iconst_3:
					case iconst_4:
					case iconst_5:
						push(opcode - iconst_0);
						index++;
						break;
					case lconst_0:
					case lconst_1:
						l = opcode - lconst_0;
						push(l);
						index++;
						break;
					case istore_0:
					case istore_1:
					case istore_2:
					case istore_3:
						b = opcode - istore_0;
						setLocal(b, popI());
						index++;
						break;
					case istore:
						i2 = (u4) code->code[index + 1];
						setLocal(i2, popI());
						index += 2;
						break;
					case iastore:
					case castore:
						v = top(); pop(); // value
						i2 = popI(); // index
						ref = popRef();
						ObjectHandler::instance()->assignArrayElement(ref, i2, &v);
						index++;
						break;
					case iload_0:
					case iload_1:
					case iload_2:
					case iload_3:
						b = opcode - iload_0;
						push(getLocalI(b));
						index++;
						break;
					case iload:
						i2 = (u4) code->code[index + 1];
						push(getLocalI(i2));
						index += 2;
						break;
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
					case bipush:
						push((int) code->code[index + 1]);
						index += 2;
						break;
					case if_icmpge:
						b = popI();
						a = popI();
						if (a >= b) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case if_icmpne:
						b = popI();
						a = popI();
						if (a != b) {
							branch1 =(char)code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case ifge:
						a = popI();
						if (a>=0) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case ifeq:
						a = popI();
						if (!a) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case ifne:
						a = popI();
						if (a) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case ifnull:
						ref = popRef();
						if (!ref) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case ifnonnull:
						ref = popRef();
						if (ref) {
							branch1 = (char) code->code[index + 1];
							branch2 = (unsigned char)code->code[index + 2];
							index += (branch1 << 8) | branch2;
						} else
							index += 3;
						break;
					case iadd:
						b = popI();
						a = popI();
						push(a + b);
						index++;
						break;
					case isub:
						b = popI();
						a = popI();
						push(a - b);
						index++;
						break;
					case imul:
						b = popI();
						a = popI();
						push(a * b);
						index++;
						break;
					case idiv:
						b = popI();
						a = popI();
						push(a / b);
						index++;
						break;
					case iinc:
						a = code->code[index + 2];
						b = getLocalI((unsigned char) code->code[index + 1]);
						setLocal((unsigned char) code->code[index + 1], b + a);
						index += 3;
						break;
					case op_goto:
						branch1 = (u4) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						index += (branch1 << 8) | branch2;
						break;
					case invokestatic:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char) code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						generateStaticCall(cf, i2, code);
						index += 3;
						break;
					case invokespecial:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						invokeSpecial(cf, i2, code);
						index += 3;
						break;
					case invokevirtual: // FIXME like the other for now, this is wrong
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						invokeSpecial(cf, i2, code);
						index += 3;
						break;
					case op_return:
						//tStack = p2;
						return;
					case ireturn:
						//a = executionStack[--tStack];
						//tStack = p2;
						//executionStack[tStack++] = a;
						return;
						break;
					case op_new:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						createNewObject(cf, i2);
						index += 3;
						break;
					case op_newarray:
						i2 = code->code[index + 1];
						createNewRawArray(i2);
						index += 2;
						break;
					case arraylength:
						i2 = ObjectHandler::instance()->getArrayLength(popRef());
						push(i2);
						index++;
						break;
					case op_dup:
						v = top();
						type = topType();
						push(v, type);
						index++;
						break;
					case putfield:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char) code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldAccess(cf, i2);
						index += 3;
						break;
					case putstatic:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldStaticAccess(cf, i2);
						index += 3;
						break;
					case getfield:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldAccess(cf, i2, false);
						index += 3;
						break;
					case getstatic:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						fieldStaticAccess(cf, i2, false);
						index += 3;
						break;
					case ldc:
						i2 = (unsigned char)code->code[index + 1];
						putConstantInStack(cf, i2, code);
						index += 2;
						break;
					case sipush:
						branch1 = (unsigned char) code->code[index + 1];
						branch2 = (unsigned char)code->code[index + 2];
						i2 = (branch1 << 8) | branch2;
						push(i2);
						index += 3;
						break;
					default:
						cerr << "Unknown opcode " << (unsigned) (unsigned char) code->code[index]
								<< " at " << cf->getClassName() << ":" << method_name << ":" << index << endl;
						throw new exception();
						break;
				} // switch
			} // while
			//tStack = p2;
		}
	}

	void JvmInterpreter::fieldAccess(ClassFile* cf, u2 fieldRef, bool toWrite) {
		CONSTANT_Fieldref_info* ci = dynamic_cast<CONSTANT_Fieldref_info*>(cf->info[fieldRef - 1]);
		u2 class_i = ci->class_index;
		u2 name_type = ci->name_and_type_index;
		CONSTANT_Class_info* clase = dynamic_cast<CONSTANT_Class_info*>(cf->info[class_i - 1]);
		CONSTANT_NameAndType_info* name = dynamic_cast<CONSTANT_NameAndType_info*>(cf->info[name_type
				- 1]);
		string class_name = cf->getUTF(clase->name_index);
		string field_name = cf->getUTF(name->name_index);
		string fdescription = cf->getUTF(name->descriptor_index);
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

	void JvmInterpreter::fieldStaticAccess(ClassFile* cf, u2 fieldRef, bool toWrite) {
		CONSTANT_Fieldref_info* ci = dynamic_cast<CONSTANT_Fieldref_info*>(cf->info[fieldRef - 1]);
		u2 class_i = ci->class_index;
		u2 name_type = ci->name_and_type_index;
		CONSTANT_Class_info* clase = dynamic_cast<CONSTANT_Class_info*>(cf->info[class_i - 1]);
		CONSTANT_NameAndType_info* name = dynamic_cast<CONSTANT_NameAndType_info*>(cf->info[name_type
				- 1]);
		string class_name = cf->getUTF(clase->name_index);
		string field_name = cf->getUTF(name->name_index);
		string fdescription = cf->getUTF(name->descriptor_index);
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

	void JvmInterpreter::generateStaticCall(ClassFile* cf, u4 methodReference,
			CodeAttribute* caller) {
		Constant_Info * cii = cf->info[methodReference- 1];
		CONSTANT_Methodref_info* ci = dynamic_cast<CONSTANT_Methodref_info*>(cii);
		u2 class_i = ci->class_index;
		u2 name_type = ci->name_and_type_index;
		CONSTANT_Class_info* clase = dynamic_cast<CONSTANT_Class_info*>(cf->info[class_i - 1]);
		CONSTANT_NameAndType_info* name = dynamic_cast<CONSTANT_NameAndType_info*>(cf->info[name_type
				- 1]);
		string class_name = cf->getUTF(clase->name_index);
		string method_name = cf->getUTF(name->name_index);
		string method_description = cf->getUTF(name->descriptor_index);
		ClassFile* tmp = loader->getClass(class_name.c_str());
		u2 method_index = tmp->getCompatibleMethodIndex(method_name.c_str(),
				method_description.c_str());
		MethodInfo* mi = tmp->methods[method_index];
		if (mi->access_flags & ACC_NATIVE) {
			cerr << "Native method call." << endl << "Calling class : " << cf->getClassName() << endl << "Method Called : " << class_name << "." << method_name << endl;
			throw new exception();
		}
		AttributeInfo* ai = mi->attributes[0];
		CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);
		// parameters
		// Now the amazing call, jajaja
		newFrame(caller->max_locals);
		int aa = method_description.find(')');
		int count = JvmExecuter::countParameters(method_description.substr(1, aa - 1));
		// copy parameters
		RuntimeValue v;
		for (int i = 0; i < count; i++) {
			v = top();
			JavaDataType type = topType();
			pop();
			setLocal(count - i - 1, type, v);
		}
		execute(tmp, mi);
		exitFrame(caller->max_locals, code->max_locals);
	}

	void JvmInterpreter::invokeSpecial(ClassFile* cf, u4 methodReference, CodeAttribute* caller) {
		// FIXME : lot of problems here, see jvm7 reference
		CONSTANT_Methodref_info* ci = dynamic_cast<CONSTANT_Methodref_info*>(cf->info[methodReference
				- 1]);
		u2 class_i = ci->class_index;
		u2 name_type = ci->name_and_type_index;
		CONSTANT_Class_info* clase = dynamic_cast<CONSTANT_Class_info*>(cf->info[class_i - 1]);
		CONSTANT_NameAndType_info* name = dynamic_cast<CONSTANT_NameAndType_info*>(cf->info[name_type
				- 1]);
		string class_name = cf->getUTF(clase->name_index);
		string method_name = cf->getUTF(name->name_index);
		string method_description = cf->getUTF(name->descriptor_index);
		ClassFile* tmp = loadAndInit(class_name);
		u2 method_index = tmp->getCompatibleMethodIndex(method_name.c_str(),
				method_description.c_str());
		MethodInfo* mi = tmp->methods[method_index];
		AttributeInfo* ai = mi->attributes[0];
		CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);
		// parameters
		// Now the amazing call, jajaja
		newFrame(caller->max_locals);
		int aa = method_description.find(')');
		int count = JvmExecuter::countParameters(method_description.substr(1, aa - 1)) + 1;
		// copy parameters
		RuntimeValue v;
		for (int i = 0; i < count; i++) {
			v = top();
			JavaDataType type = topType();
			pop();
			setLocal(count - i - 1, type, v);
		}
		execute(tmp, mi);
		exitFrame(caller->max_locals, code->max_locals);
	}

	void JvmInterpreter::createNewObject(ClassFile* cf, u4 classRef) {
		CONSTANT_Class_info* ci = dynamic_cast<CONSTANT_Class_info*>(cf->info[classRef - 1]);
		string class_name = cf->getUTF(ci->name_index);
		ClassFile* tmp = loadAndInit(class_name);
		Clase* clase = (Clase*) (buildInMemoryClass(tmp));
		Objeto obj = ObjectHandler::instance()->newObject(clase);
		push(obj);
	}

	void JvmInterpreter::putConstantInStack(ClassFile* cf, u2 index, CodeAttribute* caller) {
		Constant_Info* ri = dynamic_cast<Constant_Info*>(cf->info[index - 1]);
		CONSTANT_Integer_info* ii = dynamic_cast<CONSTANT_Integer_info*>(ri);
		if (ii)
			push(ii->value);
		else {
			CONSTANT_String_Info* si = dynamic_cast<CONSTANT_String_Info*>(ri);
			if (si) {
				string utf = cf->getUTF(si->index);
				cout << utf << endl;
				ArrayType* t = new ArrayType("",classes["char"]);
				Objeto obj = Space::instance()->newArray(t, 0); // 0 length char array FIXME
				Space::instance()->includeRoot(&obj);


				ClassFile* cf2 = loadAndInit("java/lang/String");
				Clase* clase = (Clase*) (buildInMemoryClass(cf2));
				Objeto obj2 = ObjectHandler::instance()->newObject(clase);
				push(obj2);

				u2 i2 = cf2->getCompatibleMethodIndex("<init>","([C)V");
				MethodInfo* mi = cf2->methods[i2];
				AttributeInfo* ai = mi->attributes[0];
				CodeAttribute* code = dynamic_cast<CodeAttribute*>(ai);

				newFrame(caller->max_locals);
				setLocal(0, obj2);
				setLocal(1, obj);
				execute(cf2, mi);
				exitFrame(caller->max_locals, code->max_locals);
				Space::instance()->removeRoot(&obj);
			}
			else {
				push(0.0f);
			}
		}
	}

} /* namespace jvm */
