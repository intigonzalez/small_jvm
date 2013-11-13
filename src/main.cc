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


using namespace jvm;

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char* argv[])
{
	if (argc == 1) {
		cerr << "Usage: Program [-i|-j] class_name [classes_paths]*" << endl;
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
		   cerr << "Usage: Program  [-i|-j] class_name [classes_paths]*" << endl;
		   return -1;
	   }
	}
	ios_base::sync_with_stdio(false);
	cout << "Starting : " << sizeof(void*) << '\n';
	ClassLoader::Instance()->AddPath(".");
	for (int i = optind+1; i < argc ; i++)
		ClassLoader::Instance()->AddPath(argv[i]);

	ClassFile* cf = ClassLoader::Instance()->getClass(argv[optind]);

	if (cf == NULL) {
		cerr << "Wrong class file!!!" << '\n';
		return 1;
	}

	Space::instance()->setSpaceSize(4*1024*1024);

	JvmExecuter* exec = (useNative)?
			(new JvmJit(ClassLoader::Instance(), Space::instance())):
			((JvmExecuter*)new JvmInterpreter(ClassLoader::Instance(), Space::instance()));

	for (int i = 0 ; i < 2 ; i++) {
		int result = JvmExecuter::execute_return_int(cf,"accessingArray","(I[I)I",exec);

		cout << "Results : " << result << '\n';
	}

	ClassLoader::Release();
	return 0;
}


