/*
 * classfile.cc
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include "classfile.h"
#include "classloader.h"
#include "EBadClassFile.h"

using namespace std;


/**
 ClassFile
 */
//ClassFile::ClassFile() {
//	access_flag = 0;
//	this_class = 0;
//	super_class = 0;
//	constant_pool_count = 1;
//	interfaces_count = 0;
//	fields_count = 0;
//	methods_count = 0;
//	attributes_count = 0;
//
//	initialized = false;
//
//	AddConstantUTF8("Code");
//
//	// we must add a default contructor
//	//MethodInfo* minfo = AddMethod("<init>", "()V", ACC_PUBLIC);
//
//	// aload_0,invokespecial, 0,0 ,return
//	//u1 code[] = {aload_0,invokespecial,0,0, op_return};
//
//	//minfo->AddCode(128,10,code,sizeof(code));
//}

ClassFile::~ClassFile() {
	for (unsigned i = 0; i < info.size(); i++) {
		int tag = info[i]->tag();
		delete info[i];
		if (tag == CONSTANT_Long || tag == CONSTANT_Double)
			i++;
	}
	for (int16_t i = 0; i < fields_count; i++)
		delete fields[i];
	delete [] fields;
	for (int16_t i = 0; i < methods_count; i++)
		delete methods[i];
	delete [] methods;

	delete [] interfaces;
}

//MethodInfo* ClassFile::AddMethod(const char* name, const char* descriptor, int16_t access) {
//	MethodInfo* mi = new MethodInfo();
//	mi->name_index = AddConstantUTF8(name);
//	mi->descriptor_index = AddConstantUTF8(descriptor);
//	mi->access_flags = access;
//	mi->attributes_count = 0;
//	methods.push_back(mi);
//	methods_count++;
//	return mi;
//}

//FieldInfo* ClassFile::AddField(const char* name, int16_t description_index, int16_t access) {
//	FieldInfo* field = new FieldInfo(access, AddConstantUTF8(name), description_index);
//	fields.push_back(field);
//	fields_count++;
//	return field;
//}

//int16_t ClassFile::AddNameType(int16_t name, int16_t description) {
//	CONSTANT_NameAndType_info* i = new CONSTANT_NameAndType_info(name, description);
//	info.push_back(i);
//	constant_pool_count++;
//	return info.size();
//}
//
//int16_t ClassFile::AddMethodRef(int16_t class_index, int16_t name_and_type_index) {
//	CONSTANT_Methodref_info* i = new CONSTANT_Methodref_info(class_index, name_and_type_index);
//	info.push_back(i);
//	constant_pool_count++;
//	return info.size();
//}

void ClassFile::SetAccesesRight(int16_t access) {
	this->access_flag = access;
}

//int ClassFile::AddConstantUTF8(const char* s) {
//	Constant_Info_UTF8* utf8 = new Constant_Info_UTF8(s);
//	info.push_back(utf8); // maybe this string is in table, I should use a dictionary
//	constant_pool_count++;
//	return info.size();
//}
//
//void ClassFile::SetClass(const char* name) {
//
//	CONSTANT_Class_info* class_info = new CONSTANT_Class_info(AddConstantUTF8(name));
//	info.push_back(class_info);
//	this_class = info.size();
//	constant_pool_count++;
//}
//
//void ClassFile::SetSuperClass(const char* name) {
//	CONSTANT_Class_info* class_info = new CONSTANT_Class_info(AddConstantUTF8(name));
//	info.push_back(class_info);
//	super_class = info.size();
//	constant_pool_count++;
//}

//int16_t ClassFile::AddFieldRef(int16_t class_index, int16_t name_and_type_index) {
//	pair<int16_t, int16_t> p(class_index, name_and_type_index);
//	if (_pairs.find(p) == _pairs.end()) {
//		CONSTANT_Fieldref_info* fieldRef = new CONSTANT_Fieldref_info(class_index, name_and_type_index);
//		info.push_back(fieldRef);
//		constant_pool_count++;
//		_pairs[p] = info.size();
//	}
//	return _pairs[p];
//}

//int16_t ClassFile::AddInt(int32_t value) {
//	if (_integersConst.find(value) == _integersConst.end()) {
//		CONSTANT_Integer_info* v = new CONSTANT_Integer_info(value);
//		info.push_back(v);
//		_integersConst[value] = info.size();
//		constant_pool_count++;
//	}
//	return _integersConst[value];
//}

//int16_t ClassFile::AddFloat(float value) {
//	CONSTANT_Float_info* v = new CONSTANT_Float_info(value);
//	info.push_back(v);
//	constant_pool_count++;
//	return info.size();
//}

//void ClassFile::AddMethod(MethodInfo* m) {
//	methods.push_back(m);
//	methods_count++;
//}

ClassFile::ClassFile(const char* name) {
	initialized = false;

	// We must look for a classfile with that name
	string s = name + string(".class");
	ifstream f(s.c_str(), ios::binary);

	int32_t tmp1;
	int16_t tmp2;
	int8_t tmp3;

	Load(&tmp1, f);
	//cout << tmp1 << endl;

	Load(&tmp2, f);
	Load(&tmp2, f);

	Load(&constant_pool_count, f);
	//cout << "veamos : " << constant_pool_count << endl;
	for (int i = 0; i < constant_pool_count - 1; ++i) {
		Load(&tmp3, f);
		Constant_Info* ci;
		//cout <<  "Indice "  << i+1 << " valor " << (int)tmp3 << endl;
		switch (tmp3) {
			case CONSTANT_Class:
				ci = new CONSTANT_Class_info(f);
				break;
			case CONSTANT_Double:
				ci = new CONSTANT_Double_Info(f);
				i++;
				break;
			case CONSTANT_Fieldref:
				ci = new CONSTANT_Fieldref_info(f);
				break;
			case CONSTANT_Float:
				ci = new CONSTANT_Float_info(f);
				break;
			case CONSTANT_Integer:
				ci = new CONSTANT_Integer_info(f);
				break;
			case CONSTANT_InterfaceMethodref:
				ci = new CONSTANT_InterfaceMethodref_info(f);
				break;
			case CONSTANT_NameAndType:
				ci = new CONSTANT_NameAndType_info(f);
				break;
			case CONSTANT_String:
				ci = new CONSTANT_String_Info(f);
				break;
			case CONSTANT_Methodref:
				ci = new CONSTANT_Methodref_info(f);
				break;
			case CONSTANT_Long:
				ci = new CONSTANT_Long_Info(f);
				i++;
				break;
			case CONSTANT_InvokeDynamic:
				ci = new CONSTANT_InvokeDynamic_Info(f);
				break;
			case CONSTANT_MethodHandle:
				ci = new CONSTANT_MethodHandle_Info(f);
				break;
			case CONSTANT_MethodType:
				ci = new CONSTANT_MethodType_Info(f);
				break;
			case CONSTANT_Utf8:
				ci = new Constant_Info_UTF8(f);
				break;
			default:
				cerr << "Uppps: Incorrect tag found while loading constant pool" << endl;
				cerr << "Value : " << (int) tmp3 << endl;
				cerr << "Constant pool entries processed : " << i << endl;
				throw new EBadClassFile();
				break;
		}
		info.push_back(ci);
		if (tmp3 == CONSTANT_Long || tmp3 == CONSTANT_Double)
			info.push_back(ci);
	}

	Load(&access_flag, f);

	Load(&this_class, f);

	Load(&super_class, f);

	Load(&interfaces_count, f);

	//cout << "Class Name : " << this->getClassName() << endl;
	//cout << "constant pool : " << constant_pool_count << endl;
	interfaces = new int16_t[interfaces_count];
	for (int i = 0; i < interfaces_count; ++i)
		Load(&interfaces[i], f);

	//cout << "interfaces : " << interfaces_count << endl;

	Load(&fields_count, f);
	fields = new FieldInfo*[fields_count];
	for (int i = 0; i < fields_count; ++i)
		fields[i] = new FieldInfo(f, this);

	//cout << "fields : " << fields_count << endl;

	Load(&methods_count, f);
	methods = new MethodInfo*[methods_count];
	for (int i = 0; i < methods_count; ++i)
		methods[i] = new MethodInfo(f, this);

	//cout << "methods : " << methods_count << endl;

	Load(&attributes_count, f);
	attributes_count = 0;
	//cout << "fasgfsdfsdffffffffffffffffffffffffffffffffffffffff" << endl;
	f.close();
}

int16_t ClassFile::getUTFIndex(const char* code) {
	for (unsigned i = 0; i < info.size(); i++) {
		Constant_Info* ci = info[i];
		if (ci->tag() == CONSTANT_Utf8) {
			Constant_Info_UTF8* ciUTF = (Constant_Info_UTF8*) ci;
			if (ciUTF->Equals(code))
				return i + 1;
		}
	}
	return -1;
}

//int16_t ClassFile::AddString(const char* s) {
//	string s2(s);
//	s2 = s2.substr(1, s2.size() - 2);
//	Constant_Info* ci = new CONSTANT_String_Info(AddConstantUTF8(s2.c_str()));
//	info.push_back(ci);
//	constant_pool_count++;
//	return info.size();
//}
//
//int16_t ClassFile::AddClassReference(const char* name) {
//	Constant_Info* ci = new CONSTANT_Class_info(AddConstantUTF8(name));
//	info.push_back(ci);
//	constant_pool_count++;
//	return info.size();
//}

int16_t ClassFile::getCompatibleMethodIndex(const char* methodName, const char* description) {
	int i = 0;
	while (i < methods_count) {
		int16_t nameI = methods[i]->name_index - 1;
		int16_t descI = methods[i]->descriptor_index - 1;
		if (((Constant_Info_UTF8*) this->info[nameI])->Equals(methodName)) {
			string s = ((Constant_Info_UTF8*) this->info[descI])->Value();
			if (s == description)
				return i;
		}
		i++;
	}
	// We can not find an exact match, maybe there are some compatible method
	string d = description;
	d = d.substr(1, d.find(')') - 1);
	i = 0;
	while (i < methods_count) {
		int16_t nameI = methods[i]->name_index - 1;
		int16_t descI = methods[i]->descriptor_index - 1;
		if (((Constant_Info_UTF8*) this->info[nameI])->Equals(methodName)) {
			string s = ((Constant_Info_UTF8*) this->info[descI])->Value();
			s = s.substr(1, s.find(')') - 1);
			//cout << s << " " << d << endl;
			bool b = ClassLoader::Instance()->AreCompatibleMethods(s, d);
			if (b)
				return i;
		}
		i++;
	}
	return -1;
}

string ClassFile::getClassName() {
	if (info[this_class - 1]->tag() == CONSTANT_Class) {
		CONSTANT_Class_info* ci = (CONSTANT_Class_info*)(info[this_class - 1]);
		if (info[ci->name_index - 1]->tag() == CONSTANT_Utf8) {
			Constant_Info_UTF8* ci2 = (Constant_Info_UTF8*)(info[ci->name_index - 1]);
			return ci2->Value();
		}
	}
	return "";
}

string ClassFile::getUTF(int16_t index) {
	Constant_Info_UTF8* ci2 = (Constant_Info_UTF8*)(info[index - 1]);
	if (!ci2)
		return 0;
	return ci2->Value();
}




