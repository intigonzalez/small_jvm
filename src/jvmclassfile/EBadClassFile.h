/*
 * EBadClassFile.h
 *
 *  Created on: Sep 8, 2012
 *      Author: inti
 */

#ifndef EBADCLASSFILE_H_
#define EBADCLASSFILE_H_

#include <exception>

class EBadClassFile: public std::exception {
	public:
		EBadClassFile();
};

#endif /* EBADCLASSFILE_H_ */
