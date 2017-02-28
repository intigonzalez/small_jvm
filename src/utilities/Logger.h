/*
 * Logger.h
 *
 *  Created on: Dec 28, 2013
 *      Author: inti
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <ostream>
#include <fstream>
#include <memory>

enum LogLevel {
	Debug, Info, Warning, Error
};

static const char* texts[] = {"Debug", "Info", "Warning", "Error"};

class Logger {
private:
	std::unique_ptr<std::ofstream> stream;

	void print_impl()
	{
		(*stream) << std::endl;
//		(*stream).flush();
	}


	template<typename First, typename...Rest >
	void print_impl(First parm1, Rest...parm)
	{
	    (*stream) << parm1;
//	    (*stream).flush();
	    print_impl(parm...);
	}
	
public:
	Logger(const char* fileName);
	virtual ~Logger();

	template<LogLevel level, typename First, typename...Params >
	void log(First param1, Params...params) {
		(*stream) << "<" << texts[level] << "> : ";
//		(*stream).flush();
		print_impl(param1, params...);
	}
};

extern Logger log_inst;

#define LOG_LEVEL_ON

#ifdef LOG_LEVEL_ON

#define LOG_INF log_inst.log< LogLevel::Info >
#define LOG_DBG log_inst.log< LogLevel::Debug >
#define LOG_WRN log_inst.log< LogLevel::Warning >
#define LOG_ERR log_inst.log< LogLevel::Error >

#else

#define LOG_INF
#define LOG_DBG
#define LOG_WRN
#define LOG_ERR

#endif

#endif /* LOGGER_H_ */
