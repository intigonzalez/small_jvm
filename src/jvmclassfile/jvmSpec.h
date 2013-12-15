#ifndef __JAVAVM__
#define __JAVAVM__

#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <string.h>

#include "jvmConstants.h"

using namespace std;

class ClassFile;

inline static int8_t convert(int8_t x) {
    return x;
}

inline static int16_t convert(int16_t x) {
    x = ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8);
    return x;
}

inline static int32_t convert(int32_t x) {
    x = ((x & 0xff000000) >> 24) | ((x & 0x000000ff) << 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8);
    return x;
}

template <class T> inline void Save(T x, ofstream* s) {
    x = convert(x);
    s->write((const char*) &x, sizeof (T));
}

void Load(int8_t* x, ifstream& s);

void Load(int16_t* x, ifstream& s);

void Load(int32_t* x, ifstream& s);

enum RawArrayTypes {
	T_BOOLEA=4,
	T_CHAR=5,
	T_FLOAT=6,
	T_DOUBLE=7,
	T_BYTE=8,
	T_SHORT=9,
	T_INT=10,
	T_LONG=11
};

class Constant_Info {
protected:
	int8_t _tag;
public:
	int8_t tag() {
		return _tag;
	}

	Constant_Info(const int8_t t) : _tag(t) {}
};

class Constant_Info_UTF8 : public Constant_Info {
private:
    int16_t length;
    int8_t* bytes;
public:

    Constant_Info_UTF8(ifstream& f): Constant_Info(CONSTANT_Utf8) {
        Load(&length, f);
        bytes = new int8_t[length + 1];
        f.read((char*)bytes, length);
    }

    inline bool Equals(const char* s) {
        int n = strlen(s);
        if (length != n) return false;
        for (int i = 0; i < n; i++)
            if (bytes[i] != s[i]) return false;
        return true;
    }

    string Value() {
        string s((char*)bytes,length);
//        for (int i = 0; i < length; i++)
//            s += bytes[i];
        return s;
    }
};

/**
 * 
 */
class CONSTANT_Class_info : public Constant_Info {
public:
    int16_t name_index;
public:
    CONSTANT_Class_info(ifstream& f) : Constant_Info(CONSTANT_Class) {
        Load(&name_index, f);
    }
};

class CONSTANT_REF_info : public Constant_Info {
public:
    int16_t class_index;
    int16_t name_and_type_index;
public:
    CONSTANT_REF_info(ifstream& f, const int8_t t) : Constant_Info(t) {
        Load(&class_index, f);
        Load(&name_and_type_index, f);
    }
};

class CONSTANT_Methodref_info : public CONSTANT_REF_info {
public:
    CONSTANT_Methodref_info(ifstream& f) : CONSTANT_REF_info(f, CONSTANT_Methodref) {}
};

class CONSTANT_Fieldref_info : public CONSTANT_REF_info {
public:
    CONSTANT_Fieldref_info(ifstream& f) : CONSTANT_REF_info(f, CONSTANT_Fieldref) {}
};

class CONSTANT_InterfaceMethodref_info : public CONSTANT_REF_info {
public:
    CONSTANT_InterfaceMethodref_info(ifstream& f) : CONSTANT_REF_info(f, CONSTANT_InterfaceMethodref) {}
};

class CONSTANT_NameAndType_info : public Constant_Info {
public:
    int16_t name_index;
    int16_t descriptor_index;

    CONSTANT_NameAndType_info(ifstream& f) : Constant_Info(CONSTANT_NameAndType) {
        Load(&name_index, f);
        Load(&descriptor_index, f);
    }
};

class CONSTANT_Integer_info : public Constant_Info {
public:
    int32_t value;

    CONSTANT_Integer_info(ifstream& f): Constant_Info(CONSTANT_Integer) {
        Load(&value, f);
    }
};

class CONSTANT_Float_info : public Constant_Info {
public:
    float value;

    CONSTANT_Float_info(ifstream& f): Constant_Info(CONSTANT_Float) {
        int32_t* tmp = (int32_t*) & value;
        Load(tmp, f);
    }
};

class CONSTANT_String_Info : public Constant_Info {
public:
    int16_t index;

    CONSTANT_String_Info(ifstream& f) : Constant_Info(CONSTANT_String) {
        Load(&index, f);
    }
};

class CONSTANT_Long_Info : public Constant_Info {
public:

    CONSTANT_Long_Info(ifstream& f) : Constant_Info(CONSTANT_Long) {
        int32_t n;
        Load(&n, f);
        Load(&n, f);
    }
};

class CONSTANT_Double_Info : public Constant_Info {
public:

    CONSTANT_Double_Info(ifstream& f): Constant_Info(CONSTANT_Double) {
        int32_t n;
        Load(&n, f);
        Load(&n, f);
    }
};

class CONSTANT_MethodHandle_Info : public Constant_Info {
public:

    CONSTANT_MethodHandle_Info(ifstream& f) : Constant_Info(CONSTANT_MethodHandle) {
        int16_t n;
        int8_t m;
        Load(&m, f);
        Load(&n, f);
    }
};

class CONSTANT_MethodType_Info : public Constant_Info {
public:

    CONSTANT_MethodType_Info(ifstream& f) : Constant_Info(CONSTANT_MethodType) {
        int16_t n;
        Load(&n, f);
    }
};

class CONSTANT_InvokeDynamic_Info : public Constant_Info {
public:

    CONSTANT_InvokeDynamic_Info(ifstream& f) : Constant_Info(CONSTANT_InvokeDynamic) {
        int16_t n;
        Load(&n, f);
        Load(&n, f);
    }
};

class AttributeInfo {
public:
    int16_t attribute_name_index;
    int32_t attribute_length;

    inline AttributeInfo(int16_t ani, int32_t al) {
        attribute_name_index = ani;
        attribute_length = al;
    }

    inline AttributeInfo(ifstream& f) {
        Load(&attribute_name_index, f);
        Load(&attribute_length, f);
    };

    virtual ~AttributeInfo() {} ;
};

class CodeAttribute : public AttributeInfo {
public:

    int16_t max_stack;
    int16_t max_locals;
    int32_t code_length;
    int8_t* code;
    int16_t exception_table_length; // siempre 0
    int16_t attributes_count; // siempre 0

    CodeAttribute(ifstream& f, ClassFile* cf);

    virtual ~CodeAttribute() { delete [] code;} ;
};

class MethodInfo {
public:
    int16_t access_flags;
    int16_t name_index;
    int16_t descriptor_index;
    int16_t attributes_count;
    //vector<AttributeInfo*> attributes;
    CodeAttribute* code;

    void* address;

    void cleanCode();

    MethodInfo(ifstream& f, ClassFile* cf);
    virtual ~MethodInfo();
};

class FieldInfo {
public:
    int16_t access_flags;
    int16_t name_index;
    int16_t descriptor_index;
    int16_t attributes_count; // 0
    FieldInfo(int16_t access_flags, int16_t name_index, int16_t descriptor_index);
    FieldInfo(ifstream& f, ClassFile* cf);
};






#endif
