/*
 * classfile.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <vector>

#include "jvmSpec.h"

/**
 * Represents a Java Class File
 */
class ClassFile {
	public:
		int16_t constant_pool_count;
		vector<Constant_Info*> info; // this is the constants info table
		int16_t access_flag;
		int16_t this_class;
		int16_t super_class;
		int16_t interfaces_count;
		int16_t* interfaces;
		int16_t fields_count;
		FieldInfo** fields; // this is the fields table
		int16_t methods_count;
		MethodInfo** methods; // this is the methods table
		int16_t attributes_count;
		// this is the table of attributes
		// indicate if initialized
		bool initialized;
	public:
//		ClassFile();
		ClassFile(const char* name);

		virtual ~ClassFile();

		void SetAccesesRight(int16_t access);
		void SetClass(const char* name);
		void SetSuperClass(const char* name);

		// Requesting information
		int16_t getUTFIndex(const char * code);
		int16_t getCompatibleMethodIndex(const char* methodName, const char* description);
		string getClassName();
		string getUTF(int16_t index);
};

#endif /* CLASSFILE_H_ */
