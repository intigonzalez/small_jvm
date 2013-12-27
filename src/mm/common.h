/*
 * common.h
 *
 *  Created on: Jul 23, 2012
 *      Author: inti
 */

#ifndef COMMON_H_
#define COMMON_H_

namespace MemoryManagement {

typedef struct {
	void* clase;
	void* forwardPointer;
} _Objeto;

typedef _Objeto* Objeto;

const int BASE_OBJECT_SIZE = sizeof(_Objeto);

}


#endif /* COMMON_H_ */
