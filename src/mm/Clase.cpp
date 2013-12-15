/*
 * Clase.cpp
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#include "Clase.h"

using namespace MemoryManagement;

namespace MemoryManagement {

	void TypeVisitor::visit(Type* t) {
		if (t->IsRaw())
			onRawType(t);
		else
			onClassType(t);
	}

	Type::Type(std::string name) {
		_name = name;
		_rawType = true;
		_array = false;
	}

	Type::~Type() {
	}

	bool Type::IsRaw() {
		return _rawType;
	}

	void Type::accept(TypeVisitor* v) {
		v->visit(this);
	}

	IntType::IntType() :
			Type("int") {

	}

	IntType::~IntType() {
	}

	int IntType::size() {
		return sizeof(int);
	}

	Member::Member(std::string name, Type* type) {
		_name = name;
		_type = type;
	}

	Type* Member::getType() {
		return _type;
	}

	std::string Member::getName() {
		return _name;
	}

	Clase::Clase(std::string name) :
			Type(name) {
		_totalSize = 0;
		_rawType = false;
	}

	Clase::~Clase() {
	}

	void Clase::addMember(std::string name, Type* t) {
		int index = _members.size();
		_index[name] = index;
		_members.push_back(new Member(name, t));
		int size = sizeof(Objeto);
		if (_members[index]->getType()->IsRaw() || _members[index]->getType()->isArray())
			size = _members[index]->getType()->size();

		int offset = (index == 0) ? 0 : (_offsets[index - 1] + _sizes[index - 1]);
		_offsets.push_back(offset);
		_sizes.push_back(size);
		_totalSize += size;
	}

	int Clase::size() {
		return _totalSize;
	}

	int Clase::sizeUntil(std::string memberName) {
		int n = _index[memberName];
		return _offsets[n];
	}

	int Clase::sizeUntil(int memberIndex) {
		return _offsets[memberIndex];
	}

	int Clase::memberSize(std::string memberName) {
		int n = _index[memberName];
		return _sizes[n];
	}

	int Clase::memberSize(int index) {
		return _sizes[index];
	}

	Member* Clase::getMember(int index) {
		return _members[index];
	}

	int Clase::membersCount() {
		return _members.size();
	}

	ArrayType::ArrayType(std::string name, Type* base) :
			Type(name) {
		_baseType = base;
		_array = true;
	}

	ArrayType::~ArrayType() {
		delete _baseType;
	}

	int ArrayType::size() {
		return sizeof(Objeto); // This is going to be used in Class
	}

	JavaCharType::JavaCharType() :
			Type("char") {
	}

	int JavaCharType::size() {
		return sizeof(unsigned short);
	}

	JavaCharType::~JavaCharType() {

	}
	LongType::LongType() :
			Type("long") {
	}

	int LongType::size() {
		return sizeof(long long);
	}

	LongType::~LongType() {

	}

}

/* namespace MemoryManagement */
