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
	static int countOfParameter(std::string s);
	static int countOfParameter(ClassFile* clazz, int idxInPoolToMethodRef);
};


#endif /* JVMSPECUTILS_H_ */
