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
#include "../../utilities/SimpleStack.h"

#include <vector>
#include <stack>
#include <set>

using namespace std;

namespace jit {

JitCompiler::JitCompiler(jit::CodeSectionMemoryManager* section): codeSection(section) {

}

JitCompiler::~JitCompiler() {

}

void* JitCompiler::compile(ClassFile& cf, MethodInfo& method){
	// first generate IR with quadruplos
	Routine procedure = toQuadruplus(cf, method);
	Simplex86Generator generator;
	void* addr = generator.generate(procedure, codeSection);
	return addr;
}

void JitCompiler::cfg(ClassFile& cf, MethodInfo& method, jvm::CFG& cfg) {
	vector< pair< int, int > > bytecode2qua;
	set<int> basicBlocksStart;
	int32_t branch1;
	uint8_t branch2;

	basicBlocksStart.insert(0);

	CodeAttribute* code = method.code;
	int index = 0;
	while (index < code->code_length) {
		uint8_t opcode = (uint8_t) (code->code[index]);
		switch (opcode) {
			case aconst_null:
			case aload_0:
			case aload_1:
			case aload_2:
			case aload_3:
			case astore_0:
			case astore_1:
			case astore_2:
			case astore_3:
			case iconst_m1:
			case iconst_0:
			case iconst_1:
			case iconst_2:
			case iconst_3:
			case iconst_4:
			case iconst_5:
			case istore_0:
			case istore_1:
			case istore_2:
			case istore_3:
			case iastore:
			case castore:
			case bastore:
			case iload_0:
			case iload_1:
			case iload_2:
			case iload_3:
			case i2c:
			case i2b:
			case iaload:
			case caload:
			case baload:
			case ineg:
			case iadd:
			case isub:
			case imul:
			case idiv:
			case irem:
			case ishl:
			case ishr:
			case op_pop:
			case op_return:
			case ireturn:
			case arraylength:
			case op_dup:
				index++;
				break;
			case istore:
			case aload:
			case iload:
			case bipush:
			case ldc:
			case op_newarray:
				index += 2;
				break;
			case if_icmpge:
			case if_icmpeq:
			case if_icmple:
			case if_icmplt:
			case if_icmpgt:
			case if_icmpne:
				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				basicBlocksStart.insert(branch1);
				bytecode2qua.push_back( pair<int, int>(index, branch1));
				basicBlocksStart.insert(index + 3);
				index += 3;
				break;
			case ifne:
			case ifle:
			case iflt:
			case ifge:
			case ifeq:
			case ifgt:
				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				basicBlocksStart.insert(branch1);
				bytecode2qua.push_back( pair<int, int>(index, branch1));
				basicBlocksStart.insert(index + 3);
				index += 3;
				break;
			case iinc:
			case invokestatic:
			case invokespecial:
			case invokevirtual:
				index += 3;
				break;
			case op_goto:
				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				basicBlocksStart.insert(branch1);
				bytecode2qua.push_back( pair<int, int>(index, branch1));
				index+=3;
				break;
			case op_new:
				index += 3;
				break;
			case op_dup2:
				// FIXME : LONG and Double, see jvm specification
				index++;
				break;
			case op_dup_x2:
				// FIXME : Long and double, see jvm specification
				index++;
				break;
			case putfield:
			case putstatic:
			case getfield:
			case getstatic:
			case sipush:
			case checkcast:
				index += 3;
				break;
			default:
				cerr << "Unknown opcode " << (unsigned) (uint8_t) code->code[index] << endl;
//						<< " at " << cf->getClassName() << ":" << method_name << ":" << index << endl;
				throw runtime_error("Unkown opcode");
				break;
		} // switch
	} // while

	for (auto& label : basicBlocksStart)
		cfg.addVertex(new BasicBlock(label));

	for (const auto& p: bytecode2qua){
		int src = p.first; // src of a jump in bytecode index
		int trg = p.second; // trg of jump in bytecode index
		auto it2 = basicBlocksStart.lower_bound(src);
		if (it2 != basicBlocksStart.begin() && (*it2 != src))
			--it2;
		int idxSrc = cfg.getBBWithLabel(*it2);
		int idxTrg = cfg.getBBWithLabel(trg);
		jvm::CFG::edgeType e(idxTrg, jvm::cfg_transition_type::jmp_transition);
		cfg.edges[idxSrc].push_back(e);
	}

	for (auto& label : basicBlocksStart)
		if (label >= 3) {
			int src = label - 3; // FIXME, maybe just by chance the value is opcode goto
			uint8_t opcode = (uint8_t)method.code->code[src];
			if (opcode != op_goto) {
				// there is a natural block edge
				src = label - 1;
				std::set<int>::iterator it2 = basicBlocksStart.lower_bound(src);
				if (it2 != basicBlocksStart.begin() && (*it2 != src))
					--it2;
				int idxSrc = cfg.getBBWithLabel(*it2);
				int idxTrg = cfg.getBBWithLabel(label);
				jvm::CFG::edgeType e(idxTrg, jvm::cfg_transition_type::code_order_transition);
				cfg.edges[idxSrc].push_back(e);
			}
		}
}

jit::Routine JitCompiler::toQuadruplus(ClassFile& cf, MethodInfo& method) {
	SimpleStack<jit_value, 17> values;
	vector< pair< int, int > > bytecode2qua;
	set<int> labels;
	jit_value v1,v2,v, vTmp1, vTmp2;
//	Objeto ref;
	int32_t a, b, branch1;
	uint8_t branch2;
	int32_t i2;
	int count, count2;
	Clase* javaClass;
	std::string sTmp;

	std::string currentMethodDescriptor = cf.getUTF(method.descriptor_index);
	int argumentsCount = JVMSpecUtils::countOfParameter(currentMethodDescriptor);
	if ((method.access_flags & ACC_STATIC) == 0)
		argumentsCount++;
	jit::Routine procedure(argumentsCount);

	cfg(cf, method, procedure.cfg);

	std::vector<int> blocksStart;
	for (unsigned i = 0 ; i < procedure.cfg.nodes.size(); ++i)
		blocksStart.push_back(procedure.cfg.nodes[i]->label);

//	long long l;
	OP_QUAD oper;
	//AttributeInfo* ai = method->attributes[0];
	string method_name = cf.getUTF(method.name_index);
	CodeAttribute* code = method.code;
	if (!code) return procedure;
	int index = 0;
	uint32_t nextBlockStartIndex = 0;
	while (index < code->code_length) {
		if (nextBlockStartIndex< blocksStart.size() && blocksStart[nextBlockStartIndex] == index) {
			branch1 = procedure.cfg.getBBWithLabel(index);
			procedure.currentBB = procedure.cfg.nodes[branch1];
			nextBlockStartIndex++;
		}
		uint8_t opcode = (uint8_t) (code->code[index]);
		switch (opcode) {
			case aconst_null:
				values.push(jit_null());
				index++;
				break;
			case aload:
				b = (uint8_t)code->code[index+1];
				values.push(jit_local_field(b, ObjRef));
				index += 2;
				break;
			case aload_0:
			case aload_1:
			case aload_2:
			case aload_3:
				b = opcode - aload_0;
				values.push(jit_local_field(b, ObjRef));
				index++;
				break;
			case astore_0:
			case astore_1:
			case astore_2:
			case astore_3:
				b = opcode - astore_0;
				procedure.jit_assign_local(jit_local_field(b, jit::ObjRef), values.pop());
				index++;
				break;
//				case astore:
//					b = (uint8_t)code->code[index+1];
//					setLocal(b, popRef());
//					index += 2;
//					break;
			case iconst_m1:
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
				procedure.jit_assign_local(jit_local_field(b, jit::Integer), values.pop());
				index++;
				break;
			case istore:
				i2 = (int32_t) code->code[index + 1];
				procedure.jit_assign_local(jit_local_field(i2, jit::Integer), values.pop());
				index += 2;
				break;
			case iastore:
			case castore:
			case bastore:
				v = values.pop(); // r-value
				v2 = values.pop(); // index
				v1 = values.pop(); // array

				vTmp1 = procedure.jit_binary_operation(PLUS,
						v1,jit_constant(BASE_OBJECT_SIZE + sizeof(uint32_t)));

				switch (opcode) {
				case castore:
					count = 1;
					v.meta.type = CharType;
					break;
				case bastore:
					count = 0; // the compiler should optimize this case (future)
					v.meta.type = Byte;
					break;
				default:
					count = 2;
				}

				if (count > 0)
					vTmp2 = procedure.jit_binary_operation(SHL,
						v2, jit_constant(count)); // multiply for array element's size
				else
					vTmp2 = v2;

				vTmp1 = procedure.jit_binary_operation(PLUS,
						vTmp1, vTmp2); // final address

				procedure.jit_regular_operation(MOV_TO_ADDR,
						vTmp1, v);
				index++;
				break;
			case iload_0:
			case iload_1:
			case iload_2:
			case iload_3:
				b = opcode - iload_0;
				v1 = jit_local_field(b, Integer);
				values.push(procedure.jit_copy(v1));
				index++;
				break;
			case iload:
				i2 = (int32_t) code->code[index + 1];
				v1 = jit_local_field(i2, Integer);
				values.push(procedure.jit_copy(v1));
				index += 2;
				break;
			case i2c:
				values.push(procedure.jit_binary_operation(AND,
						values.pop(),
						jit_constant(0x0000ffff)));
				index ++;
				break;
			case i2b:
				v = procedure.jit_binary_operation(AND,
						values.pop(),
						jit_constant(0x000000ff));
				v = procedure.jit_binary_operation(SHL,
						v, jit_constant(24));
				v = procedure.jit_binary_operation(SAR,
						v, jit_constant(24));
				values.push(v); // truncate
				index++;
				break;
			case bipush:
				values.push(jit_constant((int) code->code[index + 1]));
				index += 2;
				break;
			case if_icmpge:
			case if_icmpeq:
			case if_icmple:
			case if_icmplt:
			case if_icmpgt:
			case if_icmpne:
				v2 = values.pop(); // b
				v1 = values.pop(); // a
				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				labels.insert(branch1);
				oper = JGE;
				if (opcode == if_icmple) oper = JLE;
				else if (opcode == if_icmplt) oper = JLT;
				else if (opcode == if_icmpgt) oper = JG;
				else if (opcode == if_icmpeq) oper = JEQ;
				else if (opcode == if_icmpne) oper = JNE;
				procedure.jit_regular_operation(oper, v1,v2, jit_label(branch1));
				index += 3;
				break;
			case ifne:
			case ifle:
			case iflt:
			case ifge:
			case ifeq:
			case ifgt:
				v1 = values.pop();
				oper = JNE;
				if (opcode == ifle) oper = JLE;
				else if (opcode == ifge) oper = JGE;
				else if (opcode == iflt) oper = JLT;
				else if (opcode == ifeq) oper = JEQ;
				else if (opcode == ifeq) oper = JG;

				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				labels.insert(branch1);
				procedure.jit_regular_operation(oper, v1, jit_constant(0), jit_label(branch1));
				index += 3;
				break;
//				case ifnull:
//					ref = popRef();
//					if (!ref) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (uint8_t)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
//				case ifnonnull:
//					ref = popRef();
//					if (ref) {
//						branch1 = (char) code->code[index + 1];
//						branch2 = (uint8_t)code->code[index + 2];
//						index += (branch1 << 8) | branch2;
//					} else
//						index += 3;
//					break;
			case iaload:
			case caload:
			case baload:
				v2 = values.pop(); // index
				v1 = values.pop(); // address

				vTmp1 = procedure.jit_binary_operation(PLUS,
						v1,jit_constant(BASE_OBJECT_SIZE + sizeof(uint32_t)));

				count = 2;
				if (opcode == caload) {
					count = 1;
				} else if (opcode == baload)
					count = 0;

				if (count > 0)
					vTmp2 = procedure.jit_binary_operation(SHL,
						v2, jit_constant(count)); // multiply for array element's size
				else
					vTmp2 = v2;

				vTmp1 = procedure.jit_binary_operation(PLUS,
						vTmp1, vTmp2); // final address

				if (opcode == caload)
					values.push(procedure.jit_regular_operation(MOV_FROM_ADDR,
						vTmp1, useless_value, CharType));
				else if (opcode == baload)
					values.push(procedure.jit_regular_operation(MOV_FROM_ADDR,
						vTmp1, useless_value, Byte));
				else
					values.push(procedure.jit_regular_operation(MOV_FROM_ADDR,
						vTmp1, useless_value, Integer));
				index++;
				break;
			case ineg:
				v1 = values.pop();
				values.push(procedure.jit_binary_operation(SUB, jit_constant(0),v1));
				index++;
				break;
			case iadd:
			case isub:
			case imul:
			case idiv:
			case irem:
			case ishl:
			case ishr:
				v2 = values.pop();
				v1 = values.pop();
				oper = PLUS;
				if (opcode == isub) oper = SUB;
				else if (opcode == imul) oper = MUL;
				else if (opcode == idiv) oper = DIV;
				else if (opcode == irem) oper = REM;
				else if (opcode == ishl) oper = SHL;
				else if (opcode == ishr) oper = SHR;

				values.push(procedure.jit_binary_operation(oper, v1,v2));
				index++;
				break;
			case iinc:
				a = code->code[index + 2];
				v2 = jit_constant(a);
				b = (uint8_t) code->code[index + 1];
				v1 = jit_local_field(b, Integer);
				procedure.jit_regular_operation(IINC, v1,v2, useless_value);
				index += 3;
				break;
			case op_goto:
				branch1 = (char) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				branch1 = index + ((branch1 << 8) | branch2);
				procedure.jit_regular_operation(GOTO, jit::useless_value,jit::useless_value, jit_label(branch1));
				labels.insert(branch1);
				index+=3;
				break;
			case invokestatic:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t) code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2);
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.pop());
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				{
					auto tmp = getMethodToCall(cf, i2);
					if (tmp.address) {
						// the method is compiled
						// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
						values.push(procedure.jit_regular_operation(
								PLAIN_CALL,
								jit_address(tmp.address),
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
								jit_address(task),
								jit_constant(count2),
								Integer));
					}
				}
				index += 3;
				break;
			case invokespecial:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2) + 1; // +1 because of this
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.pop());
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				{
					auto tmp = getMethodToCall(cf, i2);
					if (tmp.address) {
						// the method is compiled
						// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
						procedure.jit_regular_operation(
								PLAIN_CALL,
								jit_address(tmp.address),
								jit_constant(count2),
								Void);
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
						procedure.jit_regular_operation(
								CALL_STATIC,
								jit_address(task),
								jit_constant(count2),
								Void);
					}
				}
				index += 3;
				break;
			case invokevirtual:
				// FIXME I am doing something wrong because the implementation is the same
				// as invokespecial.
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// push all the parameters
				count2 = count = JVMSpecUtils::countOfParameter(cf, i2) + 1; // +1 because of this
				while (count) {
					procedure.jit_regular_operation(PUSH_ARG,
							values.pop());
					count--;
				}
				// TODO: Check if the error (inexistent method) is triggered at execution or compilation time.
				{
					auto tmp = getMethodToCall(cf, i2);
					if (tmp.address) {
						// the method is compiled
						// FIXME: Ugly assumption regarding the return type of the method. Why Integer?
						values.push(procedure.jit_regular_operation(
								PLAIN_CALL,
								jit_address(tmp.address),
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
								jit_address(task),
								jit_constant(count2),
								Integer));
					}
				}
				index += 3;
				break;
			case op_pop:
				if (!values.empty())
					values.pop();
				else
					throw(std::runtime_error("trying to execute pop opcode in empty stack"));
				index++;
				break;
			case op_return:
				procedure.jit_return_void();
				index++;
				break;
			case ireturn:
				procedure.jit_return_int(values.pop());
				index++;
				break;
			case op_new:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;

				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG,
						jit_address(cf.myAddr()));

				values.push(procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&newObject),
						jit_constant(2),
						ObjRef));
				index += 3;
				break;
			case op_newarray:
				i2 = code->code[index + 1];
				v = values.pop(); // size of the array

				procedure.jit_regular_operation(PUSH_ARG,
						v);

				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				values.push(procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&newRawArray),
						jit_constant(2),
						ArrRef));
				index += 2;
				break;
			case arraylength:
				v1 = values.pop();

				v = procedure.jit_binary_operation(PLUS,
						v1, jit_constant(BASE_OBJECT_SIZE));

				values.push(procedure.jit_regular_operation(MOV_FROM_ADDR,
						v,useless_value, Integer));
				index++;
				break;
			case op_dup:
				v = values.top();
				values.push(procedure.jit_copy(v));
				index++;
				break;
			case op_dup2:
				// FIXME : LONG and Double, see jvm specification
				v1 = values.pop();
				v2 = values.top();
				values.push(v1);
				values.push(procedure.jit_copy(v2));
				values.push(procedure.jit_copy(v1));
				index++;
				break;
			case op_dup_x2:
				// FIXME : Long and double, see jvm specification
				v1 = values.pop();
				v2 = values.pop();
				v = values.pop();
				values.push(procedure.jit_copy(v1));
				values.push(v);
				values.push(v2);
				values.push(v1);
				index++;
				break;
			case putfield:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t) code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				v = values.pop(); // value
				v2 = values.pop(); // object

				// if the class is loaded and initialized then we can calculate the position of the field
				sTmp = JVMSpecUtils::getClassNameFromFieldRef(cf, i2);
				javaClass = jvm::JvmJit::instance()->getClassType(sTmp);
				if (javaClass) {
					sTmp = JVMSpecUtils::getFieldNameFromFieldRef(cf, i2);
					count = javaClass->sizeUntil(sTmp) + BASE_OBJECT_SIZE;
					v1 = jit_constant(count);
				}
				else {
					procedure.jit_regular_operation(PUSH_ARG,
							jit_constant(i2));

					procedure.jit_regular_operation(PUSH_ARG,
							jit_address(cf.myAddr()));

					v1 = procedure.jit_regular_operation(PLAIN_CALL,
							jit_address((void*)&getFieldDisplacement),
							jit_constant(2), // two parameters
							Integer);
				}
				v1 = procedure.jit_binary_operation(PLUS,
						v2, v1); // field address

				procedure.jit_regular_operation(MOV_TO_ADDR,
						v1, v);
				index += 3;
				break;
			case putstatic:

				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;

				procedure.jit_regular_operation(PUSH_ARG,	jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG,	jit_address(cf.myAddr()));

				v1 = procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&getStaticFieldAddress),
						jit_constant(2), // two parameters
						Integer);

				v = values.pop(); // value
				procedure.jit_regular_operation(MOV_TO_ADDR, v1, v);
				index += 3;
				break;
			case getfield:
				branch1 = (uint8_t)code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;

				v2 = values.pop(); // object

				// if the class is loaded and initialized then we can calculate the position of the field
				sTmp = JVMSpecUtils::getClassNameFromFieldRef(cf, i2);
				javaClass = jvm::JvmJit::instance()->getClassType(sTmp);
				if (javaClass) {
					sTmp = JVMSpecUtils::getFieldNameFromFieldRef(cf, i2);
					count = javaClass->sizeUntil(sTmp) + BASE_OBJECT_SIZE;
					v1 = jit_constant(count);
				}
				else {
					procedure.jit_regular_operation(PUSH_ARG,
							jit_constant(i2));

					procedure.jit_regular_operation(PUSH_ARG,
							jit_address(cf.myAddr()));

					v1 = procedure.jit_regular_operation(PLAIN_CALL,
							jit_address((void*)&getFieldDisplacement),
							jit_constant(2), // two parameters
							Integer);
				}
				v1 = procedure.jit_binary_operation(PLUS,
						v2, v1); // field address

				// FIXME, the same ugly assumption regarding member's type, Why Integer???
				values.push(procedure.jit_regular_operation(
						MOV_FROM_ADDR,
						v1,
						useless_value,
						Integer));
				index += 3;
				break;
			case getstatic:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));

				procedure.jit_regular_operation(PUSH_ARG, jit_address(cf.myAddr())); // FIXME, horrible using the class object

				v1 = procedure.jit_regular_operation(PLAIN_CALL,
						jit_address((void*)&getStaticFieldAddress),
						jit_constant(2),
						Integer);
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
				i2 = (uint8_t)code->code[index + 1];
				values.push(getConstant(cf, i2, code));
				index += 2;
				break;
			case sipush:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				values.push(jit_constant(i2));
				index += 3;
				break;
			case checkcast:
				branch1 = (uint8_t) code->code[index + 1];
				branch2 = (uint8_t)code->code[index + 2];
				i2 = (branch1 << 8) | branch2;
				// the object
				procedure.jit_regular_operation(PUSH_ARG,
						values.pop()); // FIXME, may I always remove this object
				// the index
				procedure.jit_regular_operation(PUSH_ARG,
						jit_constant(i2));
				// this classfile
				procedure.jit_regular_operation(PUSH_ARG,
								jit_address(cf.myAddr()));
				// calling
				v1 = procedure.jit_regular_operation(PLAIN_CALL,
								jit_address((void*)&getFieldDisplacement),
								jit_constant(3), // three parameters
								Void);
				index+=3;
				break;
			default:
				cerr << "Unknown opcode " << (unsigned) (uint8_t) code->code[index]
						<< " at " << cf.getClassName() << ":" << method_name << ":" << index << endl;
				throw runtime_error("Unknown opcode");
				break;
		} // switch
	} // while

	// print graph
	std::ofstream file(method_name + ".txt");
	procedure.cfg.printGraphViz(file, method_name);
	return procedure;
}

jit_value JitCompiler::getConstant(ClassFile& cf, int16_t index, CodeAttribute* caller) {
	Constant_Info* ri = (Constant_Info*)(cf.info[index - 1].get());
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

MethodInfo& JitCompiler::getMethodToCall(ClassFile& cf, int16_t idx)
{
	std::string className = JVMSpecUtils::getClassNameFromMethodRef(cf, idx);

	ClassFile& otherClazz = jvm::JvmJit::instance()->getInitiatedClass(className);

	std::string methodName = JVMSpecUtils::getMethodNameFromMethodRef(cf, idx);
	std::string methodDescription = JVMSpecUtils::getMethodDescriptionFromMethodRef(cf, idx);

	int16_t index = otherClazz.getCompatibleMethodIndex(methodName,	methodDescription);
	if (index >= 0 && index < otherClazz.methods_count) {
		return otherClazz.methods[index];
	}
	throw runtime_error("Trying to compile a non-existent method");
}

} /* namespace jit */
