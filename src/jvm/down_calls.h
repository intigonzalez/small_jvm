#ifndef __DOWN_CALLS__
#define __DOWN_CALLS__

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


Objeto newObject(ClassFile* cf, int idx);


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the address of a method already loaded within some class.
 * The function compiles the method if needed.
// */
void* getAddressForLoadedMethod(int id);

void* highlevel_loadClassCompileMethodAndPath(void* job);

/**
 * Get the address of any static field referenced by idxField within clazzFile
 */
void* getStaticFieldAddress(ClassFile* clazzFile, int idxField);


/**
 * Get the address of any field in any class referenced by idxField within clazzFile
 */
int getFieldDisplacement(ClassFile* clazzFile, int idxField);

/**
 * Implement the semantic of checkcast JVM's operator
 */
void highlevel_checkcast(ClassFile* clazzFile, int idxField, Objeto obj);

/**
 * This pseudo-function modifies the calling context to fix the target address of a previously non-compiled method.
 * Long Explanation: A method is compiled the first time it is called; hence it is possible to compile a caller before
 * having the actual address of the callee. We create an stub method to solve the problem. Such a method in charge of:
 * 1 - Compile the method
 * 2 - Return the address
 * 3 - Fix the previous call to reflect the new address, this means that the stub won't be called again
 * 4 - jmp to the address
 *
 * @return - It acts as if a call to the real function has been performed.
 * Note: This function is not intended to be called from the source code. Instead, it should be called from Jitted Code.
 *
 * When this pseudo-function is called it expect that a code as the one shown below has been executed (Intel Syntax):
 *
 * push arg_n
 * ...
 * push arg_1
 * mov ecx, StubAddress
 * call ecx
 * add esp, n*4
 * ...
 * StubAddress:
 * push StubId
 * jmp getMethodAddressAndPatch
 * ...
 * getMethodAddressAndPatch:
 */
void getMethodAddressAndPatch(); // this is not a function, in fact, it is a routine in assembly

/**
 * This pseudo-function modifies the calling context to fix the target address of a previously
 * non-initiated class and method.
 * Long Explanation: A class is initiated the first time it is used and a method is compiled the
 * first time it is called; hence it is possible to compile a caller before having the actual
 * address of the callee. We create an stub method to solve the problem. Such a method in charge of:
 * 1 - 	Load the class from disk
 * 2 - 	Preload the class and metaclass in memory. Execute the class constructor (cinit) to initialize
 *	the class
 * 3 - 	Compile the method
 * 4 - 	Return the address
 * 5 - 	Fix the previous call to reflect the new address, this means that the stub won't be called again
 * 6 - 	jmp to the address
 *
 * @return - It acts as if a call to the real function has been performed.
 * Note: This function is not intended to be called from the source code. Instead, it should be called from Jitted Code.
 *
 * When this pseudo-function is called it expect that a code as the one shown below has been executed (Intel Syntax):
 *
 * push arg_n
 * ...
 * push arg_1
 * mov ecx, StubAddress
 * call ecx
 * add esp, n*4
 * ...
 * StubAddress:
 * push LoadingJob
 * jmp loadClassCompileMethodAndPath
 * ...
 * loadClassCompileMethodAndPath:
 */
void loadClassCompileMethodAndPath();
#ifdef __cplusplus
}
#endif

#endif
