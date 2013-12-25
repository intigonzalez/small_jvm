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

std::string JVMSpecUtils::getClassNameFromMethodRef(ClassFile* clazz, int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Methodref_info* ci = (CONSTANT_Methodref_info*)(cii);
	int16_t class_i = ci->class_index;
	CONSTANT_Class_info* clase = (CONSTANT_Class_info*)(clazz->info[class_i - 1]);
//	int16_t name_type = ci->name_and_type_index;
//	CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(clazz->info[name_type - 1]);
	return clazz->getUTF(clase->name_index);
}

std::string JVMSpecUtils::getMethodNameFromMethodRef(ClassFile* clazz, int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Methodref_info* ci = (CONSTANT_Methodref_info*)(cii);
	int16_t name_type = ci->name_and_type_index;
	CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(clazz->info[name_type - 1]);
	return clazz->getUTF(name->name_index);
}

std::string JVMSpecUtils::getMethodDescriptionFromMethodRef(ClassFile* clazz,
                int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Methodref_info* ci = (CONSTANT_Methodref_info*)(cii);
	int16_t name_type = ci->name_and_type_index;
	CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(clazz->info[name_type - 1]);
	return clazz->getUTF(name->descriptor_index);
}

std::string JVMSpecUtils::getClassNameFromFieldRef(ClassFile* clazz, int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Fieldref_info* ci = (CONSTANT_Fieldref_info*)(cii);
	int16_t class_i = ci->class_index;
	CONSTANT_Class_info* clase = (CONSTANT_Class_info*)(clazz->info[class_i - 1]);
	return clazz->getUTF(clase->name_index);
}

std::string JVMSpecUtils::getFieldNameFromFieldRef(ClassFile* clazz, int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Fieldref_info* ci = (CONSTANT_Fieldref_info*)(cii);
	int16_t name_type = ci->name_and_type_index;
	CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(clazz->info[name_type - 1]);
	return clazz->getUTF(name->name_index);
}

std::string JVMSpecUtils::getFieldDescriptionFromFieldRef(ClassFile* clazz,
                int ref)
{
	Constant_Info * cii = clazz->info[ref- 1];
	CONSTANT_Fieldref_info* ci = (CONSTANT_Fieldref_info*)(cii);
	int16_t name_type = ci->name_and_type_index;
	CONSTANT_NameAndType_info* name = (CONSTANT_NameAndType_info*)(clazz->info[name_type - 1]);
	return clazz->getUTF(name->descriptor_index);
}
