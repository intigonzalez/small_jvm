/*
 * JvmInterpreter.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef JVMINTERPRETER_H_
#define JVMINTERPRETER_H_

#include "JvmExecuter.h"
#include "StackHandle.h"

#include "../mm/common.h"

#include  <functional>

namespace jvm {

	class JvmInterpreter: public jvm::JvmExecuter, public jvm::StackHandle {
		public:
			virtual void initiateClass(ClassFile* cf);

			void generateStaticCall(ClassFile* cf, u4 methodReference, CodeAttribute* caller);
			void invokeSpecial(ClassFile* cf, u4 methodReference, CodeAttribute* caller);
			void fieldAccess(ClassFile* cf, u2 fieldRef, bool toWrite = true);
			void fieldStaticAccess(ClassFile* cf, u2 fieldRef, bool toWrite = true);

			void createNewObject(ClassFile* cf, u4 methodReference);
			void createNewRawArray(int type);

			void putConstantInStack(ClassFile* cf, u2 index, CodeAttribute* caller);

		public:
			JvmInterpreter(ClassLoader* loader, Space* space);
			virtual ~JvmInterpreter();
			virtual void execute(ClassFile* cf, MethodInfo* method, std::function<void(JvmExecuter*, void* addr)> fn);
	};

}

 /* namespace jvm */
#endif /* JVMINTERPRETER_H_ */
