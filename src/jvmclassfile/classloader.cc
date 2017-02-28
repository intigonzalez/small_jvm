#include "classloader.h"
#include "classfile.h"
#include "../utilities/Logger.h"


ClassLoader* ClassLoader::ms_instance = 0;

ClassLoader::~ClassLoader()
{

}

ClassLoader* ClassLoader::Instance() {
	if (ms_instance == 0) {
		ms_instance = new ClassLoader();
	}
	return ms_instance;
}

void ClassLoader::Release() {
	if (ms_instance) {
		delete ms_instance;
	}
	ms_instance = 0;
}

ClassFile& ClassLoader::getClass(const std::string& className) {
	// FIXME: Decrease the time spent in critical section

	// Observe that this operation does not compile
	std::unique_lock<std::mutex> lock(cf_mutex);
	if (_cf.find(className) == _cf.end()) {
		// Ok, If we are here it is because the class has not been loaded, so let's do it
		for (unsigned i = 0; i < _paths.size(); i++) {
			string path = _paths[i] + className;
			if (_innerExists(path + ".class")) {
				ClassFile tmp(path.c_str());
				_cf[className] = tmp;
				LOG_DBG(__FUNCTION__, ":", __LINE__, " - ", className, ":", tmp.this_class, ":", _cf[className].this_class);
				return _cf[className];
			}
		}
		throw std::runtime_error("Unexistent class: " + className);
	}
	return _cf[className];
}

void ClassLoader::AddPath(const std::string path) {
	string s(path);
	if (s[s.size() - 1] != '/')
		s += '/';
	_paths.push_back(s);
}

ClassFile& ClassLoader::getParentClass(const ClassFile& cf) {
	LOG_DBG("ClassLoader.getParentClass: " + cf.getClassName());
	if (cf.getClassName() == string("java/lang/Object"))
		throw std::runtime_error("The class Object has no parent");

	int16_t parent = cf.super_class;
	CONSTANT_Class_info* a = static_cast<CONSTANT_Class_info*>(cf.info[parent - 1].get());

	int16_t i2 = a->name_index;

	string parentName = cf.getUTF(i2);
	LOG_DBG("ClassLoader.getParentClass: " + parentName);
	return getClass(parentName);
}

bool ClassLoader::IsPackage(const std::string& name) {
	struct stat sb;
	for (unsigned i = 0; i < _paths.size(); i++) {
		string path = _paths[i] + name;
		if (stat(path.c_str(), &sb) == 0)
			return S_ISDIR(sb.st_mode);
	}
	return false;
}

bool ClassLoader::Exists(const std::string& name) {
	struct stat sb;
	for (unsigned i = 0; i < _paths.size(); i++) {
		string path1 = _paths[i] + name;
		string path2 = path1 + ".class";
		if (stat(path1.c_str(), &sb) == 0 || stat(path2.c_str(), &sb) == 0)
			return true;
	}
	return false;
}

void ClassLoader::AddClass(const std::string name, ClassFile cf) {
	std::unique_lock<std::mutex> lock(cf_mutex);
	_cf[name] = cf;
}

bool ClassLoader::IsSubclass(const std::string& subclass, const std::string& superclass) {
	string current_class = subclass;
	if (current_class == superclass)
		return true;
	//ClassFile * super = getClass(superclass.c_str());

	while (current_class != "java/lang/Object") {
		ClassFile& sub = getClass(subclass.c_str());
		int16_t parent = sub.super_class;

		auto a = static_cast<CONSTANT_Class_info*> (sub.info[parent - 1].get());

		int16_t i2 = a->name_index;

		string parentName = sub.getUTF(i2);
		if (parentName != superclass)
			current_class = parentName;
		else
			return true;
	}
	return false;
}

bool ClassLoader::_innerExists(string s) {
	struct stat sb;
	return (stat(s.c_str(), &sb) == 0);
}

bool ClassLoader::AreCompatibleTypes(string dst, string src) {
	if (src == "null" && dst[0] == 'L')
		return true;
	bool flag = false;
	switch (src[0]) {
		case 'L':
			if (dst[0] == 'L') {
				int k1 = src.find(';');
				int k2 = dst.find(';');
				string subclass = src.substr(1, k1 - 1);
				string superclass = dst.substr(1, k2 - 1);
				flag = ClassLoader::Instance()->IsSubclass(subclass, superclass);
			} else
				flag = false;
			break;
		case 'Z':
		case 'F':
			flag = (dst[0] == src[0]);
			break;
		case 'I':
			flag = (dst[0] == 'I' || dst[0] == 'F');
			break;
	} // switch
	return flag;
}

bool ClassLoader::AreCompatibleMethods(string formalParameters, string currentParameters) {
	bool error = false;
	while (!error && currentParameters.size() && formalParameters.size()) {
		int index1 = 1;
		int index2 = 1;
		switch (formalParameters[0]) {
			case 'L':
				if (currentParameters[0] == 'L') {
					int k1 = currentParameters.find(';');
					int k2 = formalParameters.find(';');
					string subclass = currentParameters.substr(1, k1 - 1);
					string superclass = formalParameters.substr(1, k2 - 1);
					error = !ClassLoader::Instance()->IsSubclass(subclass, superclass);
					if (!error) {
						index1 = k1 + 1;
						index2 = k2 + 1;
					}
				} else
					error = true;
				break;
			case 'F':
			case 'Z':
			case 'I':
				error = (currentParameters[0] != formalParameters[0]);
				break;
		} // switch
		if (!error) {
			formalParameters = formalParameters.substr(index2);
			currentParameters = currentParameters.substr(index1);
		}
	} // while
	if (!error && formalParameters.size() == 0 && currentParameters.size() == 0)
		return true;
	return false;
}
