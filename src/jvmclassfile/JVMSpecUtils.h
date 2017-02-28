/*
 * JVMSpecUtils.h
 *
 *  Created on: Dec 15, 2013
 *      Author: inti
 */

#ifndef JVMSPECUTILS_H_
#define JVMSPECUTILS_H_

#include <string>

#include "classfile.h"


/**
 * This class wraps knowledge about the virtual machine specification.
 * For instance, it knows how is the signature of methods and fields and so on.
 *
 * The class exposes an interface to access the knowledge. Such an interface answers queries.
 */
class JVMSpecUtils {
public:
	static int countOfParameter(const std::string& s);
	static int countOfParameter(const ClassFile& clazz, int idxInPoolToMethodRef);
	static std::string getClassNameFromClassRef(const ClassFile& clazz, int ref);
	static std::string getClassNameFromMethodRef(const ClassFile& clazz, int ref);
	static std::string getMethodNameFromMethodRef(const ClassFile& clazz, int ref);
	static std::string getMethodDescriptionFromMethodRef(const ClassFile& clazz, int ref);

	static std::string getClassNameFromFieldRef(const ClassFile& clazz, int ref);
	static std::string getFieldNameFromFieldRef(const ClassFile& clazz, int ref);
	static std::string getFieldDescriptionFromFieldRef(const ClassFile& clazz, int ref);
};


#endif /* JVMSPECUTILS_H_ */
