/*
 * Objeto.h
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#ifndef OBJETO_H_
#define OBJETO_H_

#include "common.h"
#include "Clase.h"
#include "Space.h"

#include <string>
#include <vector>

namespace MemoryManagement {

	class ObjectHandler {
		private:
			ObjectHandler();
		public:
			static ObjectHandler* instance();

			MemoryManagement::Objeto newObject(Clase* clase);

			void* getMemberAddress(MemoryManagement::Objeto object, std::string memberName);
			void* getMemberAddress(MemoryManagement::Objeto object, int memberIndex);

			void assign(MemoryManagement::Objeto* dst, MemoryManagement::Objeto src);

			void assignMemberValue(MemoryManagement::Objeto obj, std::string memberName, void* value);
			void getMemberValue(MemoryManagement::Objeto obj, std::string memberName, void* value);

			int getArrayLength(Objeto obj);
			void* getArrayBuffer(Objeto obj);
			void assignArrayElement(Objeto obj, int index, void* value);
			void getArrayElement(Objeto obj, int index, void* buff);
	};

} /* namespace MemoryManagement */
#endif /* OBJETO_H_ */
