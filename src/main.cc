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


using namespace jvm;

extern int optind;

int main(int argc, char* argv[])
{
	if (argc == 1) {
		cerr << "Usage: "<< argv[0] <<" [-i|-j] class_name [classes_paths]*" << endl;
		return -1;
	}
	int c = 1;
	bool useNative = false;
	while (true) {
	   c = getopt(argc, argv, "ij");
	   if (c == -1) break;
	   switch (c) {
	   case 'i':
	   case 'j':
		   useNative = (c == 'j');
		   break;
	   case '?':
	   default:
		   cerr << "Usage: "<< argv[0] <<"  [-i|-j] class_name [classes_paths]*" << endl;
		   return -1;
	   }
	}
//	ios_base::sync_with_stdio(true);
	cout << "Starting : " << sizeof(void*) << '\n';
	ClassLoader::Instance()->AddPath(".");
	for (int i = optind+1; i < argc ; i++)
		ClassLoader::Instance()->AddPath(argv[i]);

	Space::instance()->setSpaceSize(4*1024*1024);
	
	JvmExecuter* exec = JvmJit::instance(); //new JvmJit(ClassLoader::Instance(), Space::instance());

	ClassFile* cf = exec->loadAndInit(argv[optind]);// ClassLoader::Instance()->getClass(argv[optind]);

	if (cf == nullptr) {
		cerr << "Wrong class file: " << argv[optind] << '\n';
		std::exit(1);
	}

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
	cout << "Results : " << result << endl;

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

	ClassLoader::Release();
	return 0;
}


