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


ClassFile::~ClassFile() {
}

void ClassFile::SetAccesesRight(int16_t access) {
	this->access_flag = access;
}

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
		shared_ptr<Constant_Info> ci;
		//cout <<  "Indice "  << i+1 << " valor " << (int)tmp3 << endl;
		switch (tmp3) {
			case CONSTANT_Class:
				ci = std::make_shared<CONSTANT_Class_info>(f);
				break;
			case CONSTANT_Double:
				ci = std::make_shared<CONSTANT_Double_Info>(f);
				i++;
				break;
			case CONSTANT_Fieldref:
				ci = std::make_shared<CONSTANT_Fieldref_info>(f);
				break;
			case CONSTANT_Float:
				ci = std::make_shared<CONSTANT_Float_info>(f);
				break;
			case CONSTANT_Integer:
				ci = std::make_shared<CONSTANT_Integer_info>(f);
				break;
			case CONSTANT_InterfaceMethodref:
				ci = std::make_shared<CONSTANT_InterfaceMethodref_info>(f);
				break;
			case CONSTANT_NameAndType:
				ci = std::make_shared<CONSTANT_NameAndType_info>(f);
				break;
			case CONSTANT_String:
				ci = std::make_shared<CONSTANT_String_Info>(f);
				break;
			case CONSTANT_Methodref:
				ci = std::make_shared<CONSTANT_Methodref_info>(f);
				break;
			case CONSTANT_Long:
				ci = std::make_shared<CONSTANT_Long_Info>(f);
				i++;
				break;
			case CONSTANT_InvokeDynamic:
				ci = std::make_shared<CONSTANT_InvokeDynamic_Info>(f);
				break;
			case CONSTANT_MethodHandle:
				ci = std::make_shared<CONSTANT_MethodHandle_Info>(f);
				break;
			case CONSTANT_MethodType:
				ci = std::make_shared<CONSTANT_MethodType_Info>(f);
				break;
			case CONSTANT_Utf8:
				ci = std::make_shared<Constant_Info_UTF8>(f);
				break;
			default:
				cerr << "Uppps: Incorrect tag found while loading constant pool" << endl;
				cerr << "Value : " << (int) tmp3 << endl;
				cerr << "Constant pool entries processed : " << i << endl;
				throw EBadClassFile();
				break;
		}
		info.push_back(ci);
		if (tmp3 == CONSTANT_Long || tmp3 == CONSTANT_Double)
			info.push_back(ci);
	}

	Load(&access_flag, f);

	Load(&this_class, f);

	Load(&super_class, f);


	//cout << "Class Name : " << this->getClassName() << endl;
	//cout << "constant pool : " << constant_pool_count << endl;

	Load(&interfaces_count, f);
	for (int i = 0; i < interfaces_count; ++i) {
		int16_t tmp;
		Load(&tmp, f);
		interfaces.push_back(tmp);
	}

	//cout << "interfaces : " << interfaces_count << endl;

	Load(&fields_count, f);
	for (int i = 0; i < fields_count; ++i) {
		fields.emplace_back(f, this);
	}

	//cout << "fields : " << fields_count << endl;

	Load(&methods_count, f);
	for (int i = 0; i < methods_count; ++i) {
		methods.emplace_back(f, this);
	}

	//cout << "methods : " << methods_count << endl;

	Load(&attributes_count, f);
	attributes_count = 0;
	f.close();
}

int16_t ClassFile::getUTFIndex(const char* code) {
	for (unsigned i = 0; i < info.size(); i++) {
		Constant_Info* ci = info[i].get();
		if (ci->tag() == CONSTANT_Utf8) {
			Constant_Info_UTF8* ciUTF = (Constant_Info_UTF8*) ci;
			if (ciUTF->Equals(code))
				return i + 1;
		}
	}
	return -1;
}

int16_t ClassFile::getCompatibleMethodIndex(const std::string& methodName, const std::string& description) {
	int i = 0;
	while (i < methods_count) {
		int16_t nameI = methods[i].name_index - 1;
		int16_t descI = methods[i].descriptor_index - 1;
		auto name = static_cast<Constant_Info_UTF8*>(info[nameI].get());
		if (name && name->Equals(methodName.c_str())) {
			string s = static_cast<Constant_Info_UTF8*>(info[descI].get())->Value();
			if (s == description)
				return i;
		}
		i++;
	}
	// We can not find an exact match, maybe there is some compatible method
	string d = description;
	d = d.substr(1, d.find(')') - 1);
	i = 0;
	while (i < methods_count) {
		int16_t nameI = methods[i].name_index - 1;
		int16_t descI = methods[i].descriptor_index - 1;
		if (((Constant_Info_UTF8*) this->info[nameI].get())->Equals(methodName.c_str())) {
			string s = ((Constant_Info_UTF8*) this->info[descI].get())->Value();
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

std::string ClassFile::getClassName() const {
	if (info[this_class - 1]->tag() == CONSTANT_Class) {
		CONSTANT_Class_info* ci = (CONSTANT_Class_info*)(info[this_class - 1].get());
		if (info[ci->name_index - 1]->tag() == CONSTANT_Utf8) {
			Constant_Info_UTF8* ci2 = (Constant_Info_UTF8*)(info[ci->name_index - 1].get());
			return ci2->Value();
		}
	}
	throw runtime_error("Wrong index into constants for class name: " + std::to_string(this_class));
}

std::string ClassFile::getUTF(int16_t index) const{
	Constant_Info_UTF8* ci2 = (Constant_Info_UTF8*)(info[index - 1].get());
	if (!ci2)
		return 0;
	return ci2->Value();
}
