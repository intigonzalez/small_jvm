/*
 * TemporaryFile.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#include <stdlib.h>
#include <vector>

#include "TemporaryFile.h"

namespace jit {

TemporaryFile::TemporaryFile(std::string path, std::string suffix) {
	path += "/XXXXXX" + suffix;
	std::vector<char> dst_path(path.begin(), path.end());
	dst_path.push_back('\0');

	int fd = mkstemps(&dst_path[0], suffix.size());
	if(fd != -1) {
		filePath.assign(dst_path.begin(), dst_path.end() - 1);
		stream.open(path.c_str(), std::ios_base::trunc | std::ios_base::out);
		stream.close();
	}
}

TemporaryFile::~TemporaryFile() {
	stream.close();
}

} /* namespace jit */
