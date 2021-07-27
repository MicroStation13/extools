#pragma once
#ifndef __LOGGING_H // i swear to god why do i have to do this
#define __LOGGING_H
#include <string>
#include <cstdint>
#include <cstdio>
#include "../core/core.h"

enum LogLevel {
	LOGLEVEL_DEBUG,
	LOGLEVEL_INFO,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERROR,
	LOGLEVEL_FATAL
};

#define LOGGING_MINLEVEL LOGLEVEL_INFO

typedef void (*LoggingOutput)(std::string provider, std::string message, LogLevel level);

namespace Logging {
	void Init();
	void Log(std::string provider, std::string message, LogLevel level=LOGLEVEL_INFO); // Quick and dirty
	void Debug(std::string p, std::string m);
	void Warning(std::string p, std::string m);
	void Error(std::string p, std::string m);
	void Fatal(std::string p, std::string m);
	void AddOutput(LoggingOutput out);
	// because i'm lazy enough to use this
	class LogProvider {
	public:
		std::string prov;
		LogProvider(std::string provider) {
			prov = provider;
		}
		void Log(std::string message, LogLevel level=LOGLEVEL_INFO);
		void Debug(std::string m);
		void Warning(std::string m);
		void Error(std::string m);
		void Fatal(std::string m);
	};
};

#endif