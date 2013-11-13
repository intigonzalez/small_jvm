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

inline static u1 convert(u1 x) {
    return x;
}

inline static u2 convert(u2 x) {
    x = ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8);
    return x;
}

inline static u4 convert(u4 x) {
    x = ((x & 0xff000000) >> 24) | ((x & 0x000000ff) << 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8);
    return x;
}

template <class T> inline void Save(T x, ofstream* s) {
    x = convert(x);
    s->write((const char*) &x, sizeof (T));
}

void Load(u1* x, ifstream* s);

void Load(u2* x, ifstream* s);

void Load(u4* x, ifstream* s);

class Constant_Info {
public:
    virtual u1 tag() = 0;

    inline virtual void save(ofstream* file) {
        Save(tag(), file);
    }

    virtual void load(ifstream* f) = 0;
};

class Constant_Info_UTF8 : public Constant_Info {
private:
    u2 length;
    u1* bytes;
public:

    inline virtual u1 tag() {
        return CONSTANT_Utf8;
    }
    Constant_Info_UTF8(const char* s);
    virtual ~Constant_Info_UTF8();
    
    virtual void save(ofstream* file);

    inline virtual void load(ifstream* f) {
        Load(&length, f);
        bytes = new u1[length + 1];
        f->read(bytes, length);
        bytes[length] = 0;
    }

    inline bool Equals(const char* s) {
        int n = strlen(s);
        if (length != n) return false;
        for (int i = 0; i < n; i++)
            if (bytes[i] != s[i]) return false;
        return true;
    }

    inline string Value() {
        string s;
        for (int i = 0; i < length; i++)
            s += bytes[i];
        return s;
    }
};

/**
 * 
 */
class CONSTANT_Class_info : public Constant_Info {
public:
    u2 name_index;
public:

    inline virtual u1 tag() {
        return CONSTANT_Class;
    }
    CONSTANT_Class_info(u2 name_index);
    virtual void save(ofstream* file);

    inline virtual void load(ifstream* f) {
        Load(&name_index, f);
    }
};

class CONSTANT_REF_info : public Constant_Info {
public:
    u2 class_index;
    u2 name_and_type_index;
public:
    CONSTANT_REF_info(u2 class_index, u2 name_and_type_index);
    virtual void save(ofstream* file);

    inline virtual void load(ifstream* f) {
        Load(&class_index, f);
        Load(&name_and_type_index, f);
    }
};

class CONSTANT_Methodref_info : public CONSTANT_REF_info {
public:

    inline virtual u1 tag() {
        return CONSTANT_Methodref;
    }

    CONSTANT_Methodref_info(u2 class_index, u2 name_and_type_index) : CONSTANT_REF_info(class_index, name_and_type_index) {
    }
};

class CONSTANT_Fieldref_info : public CONSTANT_REF_info {
public:

    inline virtual u1 tag() {
        return CONSTANT_Fieldref;
    }

    inline CONSTANT_Fieldref_info(u2 class_index, u2 name_and_type_index) : CONSTANT_REF_info(class_index, name_and_type_index) {
    }
};

class CONSTANT_InterfaceMethodref_info : public CONSTANT_REF_info {
public:

    inline virtual u1 tag() {
        return CONSTANT_InterfaceMethodref;
    }

    inline CONSTANT_InterfaceMethodref_info(u2 class_index, u2 name_and_type_index) : CONSTANT_REF_info(class_index, name_and_type_index) {
    }
};

class CONSTANT_NameAndType_info : public Constant_Info {
public:
    u2 name_index;
    u2 descriptor_index;
public:

    inline virtual u1 tag() {
        return CONSTANT_NameAndType;
    }
    CONSTANT_NameAndType_info(u2 name_index, u2 descriptor_index);
    virtual void save(ofstream* file);

    inline virtual void load(ifstream* f) {
        Load(&name_index, f);
        Load(&descriptor_index, f);
    }
};

class CONSTANT_Integer_info : public Constant_Info {
public:
    u4 value;
public:

    inline virtual u1 tag() {
        return CONSTANT_Integer;
    }

    CONSTANT_Integer_info(u4 value) {
        this->value = value;
    }

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save(value, file);
    }

    inline virtual void load(ifstream* f) {
        Load(&value, f);
    }
};

class CONSTANT_Float_info : public Constant_Info {
private:
    float value;
public:

    inline virtual u1 tag() {
        return CONSTANT_Float;
    }

    CONSTANT_Float_info(float value) {
        this->value = value;
    }

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        u4* tmp = (u4*) & value;
        Save(*tmp, file);
    }

    inline virtual void load(ifstream* f) {
        u4* tmp = (u4*) & value;
        Load(tmp, f);
    }
};

class CONSTANT_String_Info : public Constant_Info {
public:
    u2 index;
public:

    inline virtual u1 tag() {
        return CONSTANT_String;
    }

    inline CONSTANT_String_Info(u2 index) {
        this->index = index;
    }

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save(index, file);
    }

    inline virtual void load(ifstream* f) {
        Load(&index, f);
    }
};

class CONSTANT_Long_Info : public Constant_Info {
public:

    inline virtual u1 tag() {
        return CONSTANT_Long;
    };

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save((u4) 0, file);
        Save((u4) 0, file);
    }

    inline virtual void load(ifstream* f) {
        u4 n;
        Load(&n, f);
        Load(&n, f);
    }
};

class CONSTANT_Double_Info : public Constant_Info {
public:

    inline virtual u1 tag() {
        return CONSTANT_Double;
    };

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save((u4) 0, file);
        Save((u4) 0, file);
    }

    inline virtual void load(ifstream* f) {
        u4 n;
        Load(&n, f);
        Load(&n, f);
    }
};

class CONSTANT_MethodHandle_Info : public Constant_Info {
public:

    inline virtual u1 tag() {
        return CONSTANT_MethodHandle;
    };

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save((u1) 0, file);
        Save((u2) 0, file);
    }

    inline virtual void load(ifstream* f) {
        u2 n;
        u1 m;
        Load(&m, f);
        Load(&n, f);
    }
};

class CONSTANT_MethodType_Info : public Constant_Info {
public:

    inline virtual u1 tag() {
        return CONSTANT_MethodType;
    };

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save((u2) 0, file);
    }

    inline virtual void load(ifstream* f) {
        u2 n;
        Load(&n, f);
    }
};

class CONSTANT_InvokeDynamic_Info : public Constant_Info {
public:

    inline virtual u1 tag() {
        return CONSTANT_InvokeDynamic;
    };

    inline virtual void save(ofstream* file) {
        Constant_Info::save(file);
        Save((u2) 0, file);
        Save((u2) 0, file);
    }

    inline virtual void load(ifstream* f) {
        u2 n;
        Load(&n, f);
        Load(&n, f);
    }
};

class AttributeInfo {
public:
    u2 attribute_name_index;
    u4 attribute_length;

    inline AttributeInfo(u2 ani, u4 al) {
        attribute_name_index = ani;
        attribute_length = al;
    }

    inline AttributeInfo(ifstream* f) {
        Load(&attribute_name_index, f);
        Load(&attribute_length, f);
    };

    inline virtual void save(ofstream* f) {
        Save(attribute_name_index, f);
        Save(attribute_length, f);
    }
};

class CodeAttribute : public AttributeInfo {
public:

    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    vector<u1> code;
    u2 exception_table_length; // siempre 0
    u2 attributes_count; // siempre 0

    CodeAttribute(u2 locals, u1 * code, u4 code_length);

    CodeAttribute(ifstream* f, ClassFile* cf);

    inline virtual void save(ofstream* f) {
        AttributeInfo::save(f);
        Save(max_stack, f);
        Save(max_locals, f);
        Save(code_length, f);
        for (int i = 0; i < code_length; i++) {
            Save(code[i], f);
        }
        Save(exception_table_length, f);
        Save(attributes_count, f);
    };
};

class MethodInfo {
public:
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    vector<AttributeInfo*> attributes;

    void* address;

    MethodInfo();
    MethodInfo(ifstream* f, ClassFile* cf);
    virtual ~MethodInfo();
    
    void AddCode(u2 stack, u2 locals, u1* code, u4 code_length);
    void save(ofstream* f);
};

class FieldInfo {
public:
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count; // 0
    FieldInfo(u2 access_flags, u2 name_index, u2 descriptor_index);
    FieldInfo(ifstream * f, ClassFile* cf);
    void save(ofstream* f);
};






#endif
