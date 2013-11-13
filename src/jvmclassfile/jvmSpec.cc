#include "jvmSpec.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>

#include "classfile.h"

using namespace std;

void Load(u1* x, ifstream* s) {
	s->read((char*) x, sizeof(u1));
}

void Load(u2* x, ifstream* s) {
	s->read((char*) x, sizeof(u2));
	u2 y = convert((u2) *x);
	*x = y;
}

void Load(u4* x, ifstream* s) {
	s->read((char*) x, sizeof(u4));
	u4 y = convert((u4) *x);
	*x = y;
}

CONSTANT_Class_info::CONSTANT_Class_info(u2 name_index) {
	this->name_index = name_index;
}

void CONSTANT_Class_info::save(ofstream* file) {
	Constant_Info::save(file);
	Save(name_index, file);
}

Constant_Info_UTF8::Constant_Info_UTF8(const char* s) {
	length = strlen(s);
	bytes = new u1[length + 1];
	strcpy(bytes, s); // I am copying one more character and I am assuming s is UTF8, maybe these are so many assumptions
}

Constant_Info_UTF8::~Constant_Info_UTF8() {
	delete[] bytes;
}

void Constant_Info_UTF8::save(ofstream* file) {
	Constant_Info::save(file);
	u2 tmp = convert(length);
	file->write((const char*) &tmp, sizeof(u2));
	file->write((const char*) bytes, sizeof(u1) * length);
}

CONSTANT_REF_info::CONSTANT_REF_info(u2 class_index, u2 name_and_type_index) {
	this->class_index = class_index;
	this->name_and_type_index = name_and_type_index;
}

void CONSTANT_REF_info::save(ofstream* file) {
	Constant_Info::save(file);
	Save(class_index, file);
	Save(name_and_type_index, file);
}

CONSTANT_NameAndType_info::CONSTANT_NameAndType_info(u2 name_index, u2 descriptor_index) {
	this->name_index = name_index;
	this->descriptor_index = descriptor_index;
}

void CONSTANT_NameAndType_info::save(ofstream* file) {
	Constant_Info::save(file);
	Save(name_index, file);
	Save(descriptor_index, file);
}

CodeAttribute::CodeAttribute(u2 locals, u1 * code, u4 code_length) :
		AttributeInfo(0, 0) {
	max_stack = 128;
	max_locals = locals;
	this->code_length = code_length;
	exception_table_length = 0;
	attributes_count = 0;
	this->code.resize(code_length);
	for (int i = 0; i < code_length; i++)
		this->code[i] = code[i];
	attribute_name_index = 1;
	attribute_length = 12 + code_length;
}

CodeAttribute::CodeAttribute(ifstream* f, ClassFile* cf) :
		AttributeInfo(0, 0) {
	Load(&max_stack, f);
	Load(&max_locals, f);
	Load(&code_length, f);

	this->code.resize(code_length);
	for (int i = 0; i < code_length; i++) {
		u1 tmp;
		Load(&tmp, f);
		this->code[i] = tmp;
	}
	attribute_length = 12 + code_length;

	Load(&exception_table_length, f);
	for (int i = 0; i < exception_table_length; i++) {
		u2 tmp;
		Load(&tmp, f);
		Load(&tmp, f);
		Load(&tmp, f);
		Load(&tmp, f);
		attribute_length += 8;
	}

	Load(&attributes_count, f);
	for (int i = 0; i < attributes_count; i++) {
		u2 tmp1;
		u4 tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		f->seekg(tmp2, ios_base::cur);
	}
	attributes_count = 0;
	attribute_name_index = cf->getUTFIndex("Code");

}

/**
 MethodInfo
 */

MethodInfo::MethodInfo() {
	attributes_count = 0;
	address = 0;
}

MethodInfo::MethodInfo(ifstream* f, ClassFile* cf) {
	Load(&access_flags, f);
	Load(&name_index, f);
	Load(&descriptor_index, f);
	Load(&attributes_count, f);
	this->attributes.resize(1);
	for (int i = 0; i < attributes_count; i++) {
		u2 tmp1;
		u4 tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		if (((Constant_Info_UTF8*) cf->info[tmp1 - 1])->Equals("Code"))
			this->attributes[0] = new CodeAttribute(f, cf);
		else
			f->seekg(tmp2, ios_base::cur);
	}

	attributes_count = this->attributes.size();
	address = 0;
}

MethodInfo::~MethodInfo() {
	for (int i = 0; i < attributes.size(); i++)
		delete attributes[i];
}

void MethodInfo::AddCode(u2 stack, u2 locals, u1* code, u4 code_length) {
	CodeAttribute* codeA = new CodeAttribute(locals, code, code_length);
	attributes.push_back(codeA);
	attributes_count++;
}

void MethodInfo::save(ofstream* f) {
	Save(access_flags, f);
	Save(name_index, f);
	Save(descriptor_index, f);
	Save(attributes_count, f);
	for (int i = 0; i < attributes_count; i++)
		attributes[i]->save(f);
}

/**
 * FieldInfo
 */

FieldInfo::FieldInfo(u2 access_flags, u2 name_index, u2 descriptor_index) {
	this->access_flags = access_flags;
	this->name_index = name_index;
	this->descriptor_index = descriptor_index;
	this->attributes_count = 0;
}

FieldInfo::FieldInfo(ifstream * f, ClassFile* cf) {
	Load(&access_flags, f);
	Load(&name_index, f);
	Load(&descriptor_index, f);
	Load(&attributes_count, f);
	AttributeInfo * af;
	for (int i = 0; i < attributes_count; i++) {
		u2 tmp1;
		u4 tmp2;
		Load(&tmp1, f);
		Load(&tmp2, f);
		f->seekg(tmp2, ios_base::cur);
	}

	attributes_count = 0;
}

void FieldInfo::save(ofstream* f) {
	Save(access_flags, f);
	Save(name_index, f);
	Save(descriptor_index, f);
	Save(attributes_count, f);
}



