/*
 * Clase.h
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#ifndef CLASE_H_
#define CLASE_H_

#include <string>
#include <vector>
#include <map>

#include "common.h"

using namespace MemoryManagement;


namespace MemoryManagement {

	class TypeVisitor;

	class Type {
	protected:
		std::string _name;
		bool _rawType;
		bool _array;
	public:
		Type(std::string name);
		virtual ~Type();
		virtual int size() = 0;

		bool IsRaw();

		void accept(TypeVisitor * v);

		bool isArray() const {
			return _array;
		}

		std::string getName() const {
			return _name;
		}
	};

	class IntType: public Type {
	public:
		IntType();
		virtual int size();
		virtual ~IntType();
	};

	class ByteType: public Type {
	public:
		ByteType();
		virtual int size();
		virtual ~ByteType();
	};

	class FloatType: public Type {
	public:
		FloatType();
		virtual int size();
		virtual ~FloatType();
	};

	class BoolType: public Type {
	public:
		BoolType();
		virtual int size();
		virtual ~BoolType();
	};

	class JavaCharType : public Type {
	public:
		JavaCharType();
		virtual int size();
		virtual ~JavaCharType();
	};

	class LongType : public Type {
		public:
		LongType();
			virtual int size();
			virtual ~LongType();
	};

	class Member {
		private:
			std::string _name;
			Type* _type;
		public:
			Member(std::string name, Type* type);
			Type* getType();
			std::string getName();
	};

	class Clase: public Type {
		private:
			std::vector<Member*> _members;
			std::map<std::string, int> _index;
			std::vector<int> _sizes;
			std::vector<int> _offsets;

		public:
			int _totalSize;
			Clase(std::string name);
			virtual ~Clase();

			virtual int size();
			int sizeUntil(std::string memberName);
			int sizeUntil(int memberIndex);

			void addMember(std::string name, Type* t);

			int membersCount();

			int memberSize(std::string memberName);
			int memberSize(int index);

			Member* getMember(int index);
	};

	class ArrayType: public Type {
		private:
			Type* _baseType;
		public:
			ArrayType(std::string name, Type* base);
			virtual ~ArrayType();

			virtual int size();

			Type* getBaseType() const {
				return _baseType;
			}

			void setBaseType(Type* baseType) {
				_baseType = baseType;
			}
	};

	class TypeVisitor {
		public:
			void visit(Type* t);
			virtual void onRawType(MemoryManagement::Type* t) = 0;
			virtual void onClassType(MemoryManagement::Type* t) = 0;
	};

} /* namespace MemoryManagement */
#endif /* CLASE_H_ */
