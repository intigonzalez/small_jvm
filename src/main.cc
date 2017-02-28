/*
 * main.cc
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#include <iostream>
#include <stdlib.h>
#include <getopt.h>

using namespace std;

#include "jvmclassfile/classloader.h"
#include "jvmclassfile/classfile.h"

#include "jvm/JvmInterpreter.h"
#include "jvm/JvmExecuter.h"
#include "jvm/JvmJit.h"
#include "jvm/down_calls.h"

#include "utilities/ManyToMany.h"

using namespace jvm;

extern int optind;

int main(int argc, char* argv[])
{
//	M2MRelationship<int, int> aaa;
//	aaa.add(1,222);
//	aaa.add(1,333);
//	aaa.add(1,444);
//	aaa.add(2,444);
//	aaa.add(1,444);
//	aaa.add(1,555);
//	aaa.add(2,555);
//	aaa.add(3,555);
//	aaa.add(4,555);
//	aaa.add(1,666);
//	for (M2MRelationship<int,int>::iterator2 it = aaa.begin1(1),
//			itEnd = aaa.end1(1) ; it !=itEnd ;
//			++it){
//		int x = (*it);
//		cout << x << endl;
//	}
//
//	for (M2MRelationship<int,int>::iterator1 it = aaa.begin2(555),
//			itEnd = aaa.end2(555) ; it !=itEnd ;
//			++it){
//		int x = (*it);
//		cout << x << endl;
//	}

	if (argc == 1) {
		cerr << "Usage: "<< argv[0] <<" [-t method_name] class_name [classes_paths]*" << endl;
		return -1;
	}
	int c = 1;
	bool testing = false;
	while (true) {
	   c = getopt(argc, argv, "t");
	   if (c == -1) break;
	   switch (c) {
	   case 't':
		   testing = true;
		   break;
	   case '?':
	   default:
		   cerr << "Usage: "<< argv[0] <<"  [-t method_name] class_name [classes_paths]*" << endl;
		   return -1;
	   }
	}

	ClassLoader::Instance()->AddPath(".");

	int clazzIndex = (testing)? (optind + 1) : optind;

	for (int i = clazzIndex+1; i < argc ; i++)
		ClassLoader::Instance()->AddPath(argv[i]);

	Space::instance()->setSpaceSize(4*1024*1024);
	JvmExecuter* exec = JvmJit::instance();

	string main_class(argv[clazzIndex]);
	for (auto& c: main_class) {
		if (c == '.')
			c = '/';
	}

	cerr << "Ok 1 "  << clazzIndex << " " << main_class << endl;
	ClassFile& cf = exec->loadAndInit(main_class);
	cerr << "Ok 2 "  << clazzIndex << " " << argv[clazzIndex] << endl;

	if (testing) {
		string method_name = argv[clazzIndex - 1];
		cout << "Executing class: " << main_class << ", method: " << method_name << endl;
		int result;
		try {
			JvmExecuter::execute(cf, method_name.c_str(),"()I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
				int(*a)() = (int(*)())addr;
				int r = a();
				result = r;
			});
			cout << result << endl;
		}
		catch (runtime_error e) {
			std::cerr << e.what() << '\n';
		}
	}
	else {
		int result;
		JvmExecuter::execute(cf,"testingGetField","()I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
			int(*a)() = (int(*)())addr;
			int r = a();
			result = r;
		});
		cout << "testingGetField() = " << result << endl;

		JvmExecuter::execute(cf,"main","([Ljava/lang/String;)V",(JvmJit*)exec, [] (JvmExecuter* exec, void * addr) {
			void(*a)(Objeto) = (void(*)(Objeto))addr;
			a(nullptr);
		});

		JvmExecuter::execute(cf,"main","([Ljava/lang/String;)V",(JvmJit*)exec, [] (JvmExecuter* exec, void * addr) {
			void(*a)(Objeto) = (void(*)(Objeto))addr;
			a(nullptr);
		});

		JvmExecuter::execute(cf,"accessingArray","(I[I)I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
			Objeto array = newRawArray(T_INT, 20);
			int value1 = 1;
			int value2 = 2;
			ObjectHandler::instance()->assignArrayElement(array, 0 , &value1);
			ObjectHandler::instance()->assignArrayElement(array, 1 , &value2);
			int(*a)(int, Objeto) = (int(*)(int,Objeto))addr;
			int r = a(1,array);
			result = r;
		});
		cout << "accessingArray(1, Arr[20]) : " << result << endl;

		int n = 6;
		JvmExecuter::execute(cf,"factorial","(I)I",(JvmJit*)exec, [&result,n] (JvmExecuter* exec, void * addr) {
			int(*a)(int) = (int(*)(int))addr;
			int r = a(n); // for some crazy reason I cannot assign result = a(n)
			result = r;
		});
		cout << "factorial(" << n << ") = " << result << endl;

		JvmExecuter::execute(cf,"anotherTest","()I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
			int(*a)() = (int(*)())addr;
			int r = a(); // for some crazy reason I cannot assign result = a(n)
			result = r;
		});
		cout << "anotherTest() = " << result << endl;

		JvmExecuter::execute(cf,"testingNew","()I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
			int(*a)() = (int(*)())addr;
			int r = a(); // for some crazy reason I cannot assign result = a(n)
			result = r;
		});
		cout << "testingNew() = " << result << endl;

		JvmExecuter::execute(cf,"testingNew2","()I",(JvmJit*)exec, [&result] (JvmExecuter* exec, void * addr) {
			int(*a)() = (int(*)())addr;
			int r = a(); // for some crazy reason I cannot assign result = a(n)
			result = r;
		});
		cout << "testingNew2() = " << result << endl;
	}
	ClassLoader::Release();
	return 0;
}
