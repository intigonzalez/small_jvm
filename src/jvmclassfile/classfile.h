/*
 * classfile.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <vector>
#include <memory>

#include "jvmSpec.h"

/**
 * Represents a Java Class File
 */
class ClassFile {
	public:
		int16_t constant_pool_count;
		std::vector<std::shared_ptr<Constant_Info>> info; // this is the constants info table
		int16_t access_flag;
		int16_t this_class;
		int16_t super_class;
		int16_t interfaces_count;
		std::vector<int16_t> interfaces;
		int16_t fields_count;
		std::vector<FieldInfo> fields; // this is the fields table
		int16_t methods_count;
		std::vector<MethodInfo> methods; // this is the methods table
		int16_t attributes_count;
		// this is the table of attributes
		// indicate if initialized
		bool initialized;
	public:
		ClassFile() {};
		ClassFile(const char* name);

		virtual ~ClassFile();

		void SetAccesesRight(int16_t access);
		void SetClass(const char* name);
		void SetSuperClass(const char* name);

		// Requesting information
		int16_t getUTFIndex(const char * code);
		int16_t getCompatibleMethodIndex(const std::string& methodName, const std::string& description);
		std::string getClassName() const;
		std::string getUTF(int16_t index) const;

		bool isObjectClass() {  return getClassName() == "java/lang/Object"; };

		void* myAddr() {  return this; } // FIXME: HORRIBLE
};

#endif /* CLASSFILE_H_ */
