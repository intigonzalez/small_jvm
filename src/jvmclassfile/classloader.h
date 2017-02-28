#ifndef CLASSLOADER_H
#define CLASSLOADER_H

#include <vector>
#include <map>
#include <string>
#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>
#include "jvmSpec.h"

using namespace std;

class ClassLoader {
private:
	static ClassLoader* ms_instance;

	std::mutex cf_mutex;

public:
	static ClassLoader* Instance();
	static void Release();

	void AddPath(const std::string path);

	ClassFile& getClass(const std::string& className);
	ClassFile& getParentClass(const ClassFile& cf);

	bool IsPackage(const std::string& name);

	bool Exists(const std::string& name);

	bool IsSubclass(const std::string& subclass, const std::string& superclass);

	void AddClass(const std::string name, ClassFile cf);

	bool AreCompatibleTypes(string dst, string src);

	bool AreCompatibleMethods(string formalParameters, string currentParameters);

private:
	vector<std::string> _paths;
	map<std::string, ClassFile> _cf;

	bool _innerExists(string s);

	virtual ~ClassLoader();

};

#endif // CLASSLOADER_H
