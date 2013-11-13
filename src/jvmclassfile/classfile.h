/*
 * classfile.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <vector>
#include <map>

#include "jvmSpec.h"

/**
 * Represents a Java Class File
 */
class ClassFile {
	public:
		map<pair<u2, u2>, u2> _pairs;
		map<u4, u2> _integersConst;

		u2 constant_pool_count;
		vector<Constant_Info*> info; // this is the constants info table
		u2 access_flag;
		u2 this_class;
		u2 super_class;
		u2 interfaces_count;
		vector<u2> interfaces;
		u2 fields_count;
		vector<FieldInfo*> fields; // this is the fields table
		u2 methods_count;
		vector<MethodInfo*> methods; // this is the methods table
		u2 attributes_count;
		// this is the table of attributes
		// indicate if initialized
		bool initialized;
	public:
		ClassFile();
		ClassFile(const char* name);

		virtual ~ClassFile();

		void SetAccesesRight(u2 access);
		void SetClass(const char* name);
		void SetSuperClass(const char* name);

		void save(const char * filename, bool myclass = true);

		int AddConstantUTF8(const char* s);
		FieldInfo* AddField(const char* name, u2 description_index, u2 access);
		MethodInfo* AddMethod(const char* name, const char* descriptor, u2 access);
		void AddMethod(MethodInfo* m);
		u2 AddNameType(u2 name, u2 description);
		u2 AddMethodRef(u2 class_index, u2 name_and_type_index);
		u2 AddFieldRef(u2 class_index, u2 name_and_type_index);
		u2 AddClassReference(const char* name);

		u2 AddInt(u4 value);
		u2 AddString(const char* s);
		u2 AddFloat(float value);

		// Requesting information
		u2 getUTFIndex(const char * code);
		u2 getCompatibleMethodIndex(const char* methodName, const char* description);
		string getClassName();
		string getUTF(u2 index);

		int getParameterCount(u2 description_index);

};

#endif /* CLASSFILE_H_ */
