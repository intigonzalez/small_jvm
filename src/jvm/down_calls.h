#ifndef __DOWN_CALLS__
#define __DOWN_CALLS__

#ifdef __cplusplus
//extern "C" {
#endif

#include "../mm/Objeto.h"
#include "../jvmclassfile/jvmSpec.h"

#include <map>


extern std::map<std::string, Type*> rawTypes;

void initDownCalls();

/**
 * Create a new array object
 * @type Base type of the array, it uses JVM Specification constants
 * @length Length of the new array
 */
Objeto newRawArray(RawArrayTypes type, int length);


#ifdef __cplusplus
//}
#endif

#endif
