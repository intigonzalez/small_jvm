/*
 * jni.cpp
 *
 *  Created on: Sep 24, 2012
 *      Author: inti
 */

#include "jni.h"

#include "../mm/Clase.h"
#include "../mm/Objeto.h"

#include "../jvmclassfile/classloader.h"

JNINativeInterface_ nativeInterface;
JNIEnv _env = { &nativeInterface };
JNIEnv* env;

jsize JNICALL getArrayLength(JNIEnv *, jarray a) {
	return ObjectHandler::instance()->getArrayLength(Objeto(a));
}

jchar* JNICALL getCharArrayElements(JNIEnv * e, jcharArray a, jboolean* isCopy) {
	*isCopy = JNI_FALSE;
	return (jchar*) ObjectHandler::instance()->getArrayBuffer((Objeto) a);

}

void JNICALL releaseCharArrayElements(JNIEnv * e, jcharArray a, jchar * elems, jint mode) {
	// do nothing, elems is never a copy
}

jboolean JNICALL isInstanceOf (JNIEnv * e, jobject o, jclass clazz) {
	Type* type = (Type*)clazz;
	Objeto obj = (Objeto)o;
	Type* t2 = (Type*)obj->clase;
	if (type->isArray() && t2->isArray() && type->getName() == t2->getName()) {
		return JNI_TRUE;
	}
	else if (type->isArray() && t2->isArray()) {
		type = ((ArrayType*)type)->getBaseType();
		t2 = ((ArrayType*)t2)->getBaseType();
		if (!type->IsRaw() && !t2->IsRaw() && !type->isArray() && !t2->isArray()) {
			Clase* c1 = (Clase*)type;
			Clase* c2 = (Clase*)t2;
			return ClassLoader::Instance()->IsSubclass(c2->getName(), c1->getName());
		}
	}
	else if (!type->IsRaw() && !t2->IsRaw() && !type->isArray() && !t2->isArray()) {
		Clase* c1 = (Clase*)type;
		Clase* c2 = (Clase*)t2;
		return ClassLoader::Instance()->IsSubclass(c2->getName(), c1->getName());
	}
	return JNI_FALSE;
}

void initJNI() {
	nativeInterface.GetArrayLength = getArrayLength;
	nativeInterface.GetCharArrayElements = getCharArrayElements;
	nativeInterface.ReleaseCharArrayElements = releaseCharArrayElements;
	nativeInterface.IsInstanceOf = isInstanceOf;
}

