#include "jvmSpec.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>

#include "classfile.h"

using namespace std;

void Load(int8_t* x, ifstream& s) {
	s.read((char*) x, sizeof(int8_t));
}

void Load(int16_t* x, ifstream& s) {
	s.read((char*) x, sizeof(int16_t));
	int16_t y = convert((int16_t) *x);
	*x = y;
}

void Load(int32_t* x, ifstream& s) {
	s.read((char*) x, sizeof(int32_t));
	int32_t y = convert((int32_t) *x);
	*x = y;
}

//CONSTANT_Class_info::CONSTANT_Class_info(int16_t name_index) {
//	this->name_index = name_index;
//}
//
//Constant_Info_UTF8::Constant_Info_UTF8(const char* s) {
//	length = strlen(s);
//	bytes = new int8_t[length + 1];
//	strcpy(bytes, s); // I am copying one more character and I am assuming s is UTF8, maybe these are so many assumptions
//}
//
//Constant_Info_UTF8::~Constant_Info_UTF8() {
//	delete[] bytes;
//}
//
//CONSTANT_REF_info::CONSTANT_REF_info(int16_t class_index, int16_t name_and_type_index) {
//	this->class_index = class_index;
//	this->name_and_type_index = name_and_type_index;
//}
//
//CONSTANT_NameAndType_info::CONSTANT_NameAndType_info(int16_t name_index, int16_t descriptor_index) {
//	this->name_index = name_index;
//	this->descriptor_index = descriptor_index;
//}

CodeAttribute::CodeAttribute(ifstream& f, ClassFile* cf) :
		AttributeInfo(0, 0) {
	Load(&max_stack, f);
	Load(&max_locals, f);
	Load(&code_length, f);

	this->code = new int8_t[code_length];
	f.read((char*)this->code, code_length);
//	for (int i = 0; i < code_length; i++) {
//		int8_t tmp;
//		Load(&tmp, f);
//		this->code[i] = tmp;
//	}
	attribute_length = 12 + code_length;

	Load(&exception_table_length, f);
	for (int i = 0; i < exception_table_length; i++) {
		int16_t tmp;
		Load(&tmp, f);
		Load(&tmp, f);
		Load(&tmp, f);
		Load(&tmp, f);
		attribute_length += 8;
	}

	Load(&attributes_count, f);
	for (int i = 0; i < attributes_count; i++) {
		int16_t tmp1;
		int32_t tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		f.seekg(tmp2, ios_base::cur);
	}
	attributes_count = 0;
	attribute_name_index = cf->getUTFIndex("Code");

}

/**
 MethodInfo
 */

MethodInfo::MethodInfo(ifstream& f, ClassFile* cf) {
	Load(&access_flags, f);
	Load(&name_index, f);
	Load(&descriptor_index, f);
	Load(&attributes_count, f);
	this->code = nullptr;
	//this->attributes.resize(1);
	for (int i = 0; i < attributes_count; i++) {
		int16_t tmp1;
		int32_t tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		if (((Constant_Info_UTF8*) cf->info[tmp1 - 1].get())->Equals("Code"))
			this->code = new CodeAttribute(f, cf);
		else
			f.seekg(tmp2, ios_base::cur);
	}

	if (this->code)
		attributes_count = 1;//this->attributes.size();
	address = 0;
}

MethodInfo::~MethodInfo() {
//	for (unsigned i = 0; i < attributes.size(); i++)
//		delete attributes[i];
	cleanCode();
}

void MethodInfo::cleanCode() {
	if (!code)
		delete code;
	code = nullptr;
}

/**
 * FieldInfo
 */
FieldInfo::FieldInfo(int16_t access_flags, int16_t name_index, int16_t descriptor_index) {
	this->access_flags = access_flags;
	this->name_index = name_index;
	this->descriptor_index = descriptor_index;
	this->attributes_count = 0;
}

FieldInfo::FieldInfo(ifstream& f, ClassFile* cf) {
	Load(&access_flags, f);
	Load(&name_index, f);
	Load(&descriptor_index, f);
	Load(&attributes_count, f);
	// AttributeInfo * af;
	for (int i = 0; i < attributes_count; i++) {
		int16_t tmp1;
		int32_t tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		f.seekg(tmp2, ios_base::cur);
	}

	attributes_count = 0;
}
