/*
 * CodeSectionMemoryManager.h
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#ifndef CODESECTIONMEMORYMANAGER_H_
#define CODESECTIONMEMORYMANAGER_H_

#include <stdio.h>

namespace jit {

class CodeSectionMemoryManager {
private:
	void* baseAddr;
	size_t size;

	void* currentAddr;
	size_t free;
public:
	CodeSectionMemoryManager(size_t size);
	virtual ~CodeSectionMemoryManager();

	void* nextAddress() { return baseAddr; }
	void* getChunck(size_t s);
};

} /* namespace jit */

#endif /* CODESECTIONMEMORYMANAGER_H_ */
