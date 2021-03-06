/*
 * CodeSectionMemoryManager.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#include "CodeSectionMemoryManager.h"

#include <sys/mman.h>
#include <exception>
#include <stdexcept>

namespace jit {

CodeSectionMemoryManager::CodeSectionMemoryManager(size_t size) {
	this->size = size;
	this->free = size;
	baseAddr = mmap(nullptr, this->size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	currentAddr = baseAddr;
	if (baseAddr == MAP_FAILED) {
		throw std::runtime_error("Failed memory mapping");
	}
}

CodeSectionMemoryManager::~CodeSectionMemoryManager() {
//	munmap(baseAddr, this->size);
}

void* CodeSectionMemoryManager::getChunck(size_t s) {
	if (s < free) {
		free -= s;
		char* tmp = (char*)currentAddr;
		void* r = currentAddr;
		currentAddr = &tmp[s];
		return r;
	}
	return nullptr;
}

} /* namespace jit */
