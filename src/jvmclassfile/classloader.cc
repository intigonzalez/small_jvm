#include "classloader.h"
#include "classfile.h"

ClassLoader* ClassLoader::ms_instance = 0;

ClassLoader::ClassLoader() {
}

ClassLoader::~ClassLoader() {
	for (map<string, ClassFile*>::iterator it = _cf.begin(); it != _cf.end(); it++) {
		delete it->second;
	}
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

ClassFile* ClassLoader::getClass(const char* className) {
	if (_cf.find(className) == _cf.end()) {

		for (unsigned i = 0; i < _paths.size(); i++) {
			string path = _paths[i] + className;
			if (_innerExists(path + ".class")) {
				_cf[className] = new ClassFile(path.c_str());
				return _cf[className];
			}
		}
		return 0;
	}
	return _cf[className];
}

void ClassLoader::AddPath(const char* path) {
	string s(path);
	if (s[s.size() - 1] != '/')
		s += '/';
	_paths.push_back(s);
}

ClassFile* ClassLoader::getParentClass(ClassFile* cf) {
	if (cf->getClassName() == string("java/lang/Object"))
		return 0;
	int16_t parent = cf->super_class;

	CONSTANT_Class_info* a = (CONSTANT_Class_info*) cf->info[parent - 1];

	int16_t i2 = a->name_index;

	string parentName = cf->getUTF(i2);
	return getClass(parentName.c_str());
}

bool ClassLoader::IsPackage(const char* name) {
	struct stat sb;
	for (unsigned i = 0; i < _paths.size(); i++) {
		string path = _paths[i] + name;
		if (stat(path.c_str(), &sb) == 0)
			return S_ISDIR(sb.st_mode);
	}
	return false;
}

bool ClassLoader::Exists(string name) {
	struct stat sb;
	for (unsigned i = 0; i < _paths.size(); i++) {
		string path1 = _paths[i] + name;
		string path2 = path1 + ".class";
		if (stat(path1.c_str(), &sb) == 0 || stat(path2.c_str(), &sb) == 0)
			return true;
	}
	return false;
}

void ClassLoader::AddClass(string name, ClassFile* cf) {
	_cf[name] = cf;
}

bool ClassLoader::IsSubclass(string subclass, string superclass) {
	if (subclass == superclass)
		return true;
	//ClassFile * super = getClass(superclass.c_str());

	while (subclass != "java/lang/Object") {
		ClassFile * sub = getClass(subclass.c_str());
		int16_t parent = sub->super_class;

		CONSTANT_Class_info* a = (CONSTANT_Class_info*) sub->info[parent - 1];

		int16_t i2 = a->name_index;

		string parentName = sub->getUTF(i2);
		if (parentName != superclass)
			subclass = parentName;
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
