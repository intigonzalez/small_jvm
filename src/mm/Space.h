/*
 * Space.h
 *
 *  Created on: Jul 22, 2012
 *      Author: inti
 */

#ifndef SPACE_H_
#define SPACE_H_

#include "Objeto.h"


#include <iterator>

#include <set>

#include <queue>

using namespace std;

namespace MemoryManagement {

	class RootsProvider {
		public:
			virtual bool hasNext() = 0;
			virtual Objeto* next() = 0;
	};

	class Space {
		private:

			char* addressInmortal;
			char* firstInmortal;
			int freeInmortalSpace;

			char* address;

			char* mark0;
			char* mark1;

			char* firstFree;

			std::vector<MemoryManagement::Objeto*> roots;
			RootsProvider* extraRoots;

			int freeSpace;
			int halfspace;

			Space(long size);

			MemoryManagement::Objeto fastAlloc(Clase* clase, int n);

			void* explore(Objeto obj);

			void copy(Objeto obj);

		protected:
			void doCollection();
		public:
			long _totalTime;

			virtual ~Space();
			static Space* instance();

			void setSpaceSize(int size);

			MemoryManagement::Objeto newObject(Clase* clase);

			MemoryManagement::Objeto newArray(ArrayType* a, int length);

			void includeRoot(MemoryManagement::Objeto* obj);
			void removeRoot(MemoryManagement::Objeto* obj);
			void removeRoots(int count);

			void setExtraRootsProvider(RootsProvider* provider);
	};

} /* namespace MemoryManagement */
#endif /* SPACE_H_ */
