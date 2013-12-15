/*
 * TemporaryFile.h
 *
 *  Created on: Nov 17, 2013
 *      Author: inti
 */

#ifndef TEMPORARYFILE_H_
#define TEMPORARYFILE_H_

#include <string>
#include <fstream>

namespace jit {

class TemporaryFile {
private:
	std::ofstream stream;
	std::string filePath;
public:
	TemporaryFile(std::string path, std::string suffix);
	virtual ~TemporaryFile();

	std::ostream& getFile() { return stream; } ;

	const std::string getFilePath() { return filePath; }

	void open() {
		stream.open(filePath.c_str());
	}

	void close() {
		stream.close();
	}
};

} /* namespace jit */

#endif /* TEMPORARYFILE_H_ */
