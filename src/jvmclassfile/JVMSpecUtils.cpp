/*
 * JVMSpecUtils.cpp
 *
 *  Created on: Dec 15, 2013
 *      Author: inti
 */

#include "JVMSpecUtils.h"


int JVMSpecUtils::countOfParameter(std::string s) {
	size_t pos = 1;
	int count = 0;
	while (s[pos] != ')') {
		switch (s[pos]) {
			case '[':
				pos++;
				break;
			case 'L':
				pos = s.find(';', pos) + 1;
				count++;
				break;
			default:
				count++;
				pos++;
				break;
		}
	}
	return count;
}

int JVMSpecUtils::countOfParameter(ClassFile* clazz, int idxInPoolToMethodRef) {
	Constant_Info* info = clazz->info[idxInPoolToMethodRef-1];
	if (info->tag() == CONSTANT_Methodref) {
		CONSTANT_Methodref_info* mRef = (CONSTANT_Methodref_info*) info;
		int16_t idx2 = mRef->name_and_type_index;
		CONSTANT_NameAndType_info* name_type = (CONSTANT_NameAndType_info*)clazz->info[idx2-1];
		idx2 = name_type->descriptor_index;
		std::string signature = clazz->getUTF(idx2);
		return countOfParameter(signature);
	}
	return 0;
}

