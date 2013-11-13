#ifndef CLASSLOADER_H
#define CLASSLOADER_H

#include <vector>
#include <map>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include "jvmSpec.h"

using namespace std;

class ClassLoader {
		static ClassLoader* ms_instance;

	public:
		static ClassLoader* Instance();
		static void Release();

		void AddPath(const char* path);

		ClassFile* getClass(const char* className);

		ClassFile* getParentClass(ClassFile* cf);

		bool IsPackage(const char* name);

		bool Exists(string name);

		bool IsSubclass(string subclass, string superclass);

		void AddClass(string name, ClassFile* cf);

		bool AreCompatibleTypes(string dst, string src);

		bool AreCompatibleMethods(string formalParameters, string currentParameters);

	private:

		vector<string> _paths;
		map<string, ClassFile*> _cf;

		bool _innerExists(string s);

		ClassLoader();
		virtual ~ClassLoader();

};

#endif // CLASSLOADER_H
