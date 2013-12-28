/*
 * Logger.cpp
 *
 *  Created on: Dec 28, 2013
 *      Author: inti
 */

#include "Logger.h"

Logger::Logger(const char* fileName):stream(new std::ofstream)
{
	stream->open(fileName, std::ios_base::out);
	if( !stream->is_open() )
	{
		throw(std::runtime_error("LOGGER: Unable to open an output stream"));
	}
}

Logger::~Logger()
{
	if (stream)
		stream->close();
}
