#include "logging.h"
#include <ctime>
#include <cstring>
bool LogsInit = false;
std::vector<LoggingOutput> logout;

void Logging::AddOutput(LoggingOutput out) {
	logout.push_back(out);
}

std::string pt_levels[5] = {
	"DEBUG",
	"INFO ",
	"WARN ",
	"ERROR",
	"FATAL"
};

#ifdef _WIN32
auto levels = pt_levels;
#else
const char * levels[5] = {
	"DEBUG",
	"\x1b[36mINFO \x1b[0m",
	"\x1b[33mWARN \x1b[0m",
	"\x1b[31mERROR\x1b[0m",
	"\x1b[5;31mFATAL\x1b[0m"
};
#endif

char * get_ts() {
	std::time_t result = std::time(nullptr);
	char * ts = std::asctime(std::localtime(&result));
	int idx = strlen(ts);
	ts[idx-1] = 0;
	return ts;
}

void log_tty(std::string provider, std::string message, LogLevel level) {
	printf("[%s] [%s] [%s] %s\n", get_ts(), levels[level], provider.c_str(), message.c_str());
}

void log_world(std::string provider, std::string message, LogLevel level) {
	std::string msg = "[";
	msg += get_ts();
	msg += "] [" + pt_levels[level] + "] [" + provider + "] " + message;
	Core::get_proc("/proc/log_world").call({Value(msg)});
}

FILE * logfile;

void log_file(std::string provider, std::string message, LogLevel level) {
	fprintf(logfile, "[%s] [%s] [%s] %s\n", get_ts(), pt_levels[level].c_str(), provider.c_str(), message.c_str());
}

void Logging::Log(std::string provider, std::string message, LogLevel level) {
	if (!LogsInit) Init();
	if (level < LOGGING_MINLEVEL) return;
	for (LoggingOutput o : logout) {
		o(provider, message, level);
	}
}

void Logging::Debug(std::string p, std::string m) {
	Log(p, m, LOGLEVEL_DEBUG);
}
void Logging::Warning(std::string p, std::string m) {
	Log(p, m, LOGLEVEL_WARNING);
}
void Logging::Error(std::string p, std::string m) {
	Log(p, m, LOGLEVEL_ERROR);
}
void Logging::Fatal(std::string p, std::string m) {
	Log(p, m, LOGLEVEL_FATAL);
}

trvh dm_log(unsigned int args_len, Value* args, Value src) {
	if (args[0].type != DataType::STRING or args[1].type != DataType::STRING)
		Runtime("Expected string");
	Logging::Log(args[0], args[1]);
}

trvh dm_debug(unsigned int args_len, Value* args, Value src) {
	if (args[0].type != DataType::STRING or args[1].type != DataType::STRING)
		Runtime("Expected string");
	Logging::Debug(args[0], args[1]);
}

trvh dm_warning(unsigned int args_len, Value* args, Value src) {
	if (args[0].type != DataType::STRING or args[1].type != DataType::STRING)
		Runtime("Expected string");
	Logging::Warning(args[0], args[1]);
}

trvh dm_error(unsigned int args_len, Value* args, Value src) {
	if (args[0].type != DataType::STRING or args[1].type != DataType::STRING)
		Runtime("Expected string");
	Logging::Error(args[0], args[1]);
}

trvh dm_fatal(unsigned int args_len, Value* args, Value src) {
	if (args[0].type != DataType::STRING or args[1].type != DataType::STRING)
		Runtime("Expected string");
	Logging::Fatal(args[0], args[1]);
}

void l_try_hook(std::string proc, ProcHook hook) {
	auto p_ptr = Core::try_get_proc(proc);
	if (p_ptr != nullptr) {
		(*p_ptr).hook(hook);
	} else {
		Logging::Warning("Logging", "Failed to hook proc "+proc);
	}
}

void Logging::Init() {
	logfile = fopen("luatools.log", "a");
	fprintf(logfile, "\n=============================================================\n");
	fprintf(logfile, "                  Luatools logging started                   \n");
	fprintf(logfile, "=============================================================\n");
	#ifdef _WIN32
	AddOutput(log_world);
	#else
	AddOutput(log_tty);
	#endif
	AddOutput(log_file);
	LogsInit = true;
	Log("Logging", "Logging init done!");
	l_try_hook("/proc/console_log", dm_log);
	l_try_hook("/proc/console_warning", dm_warning);
	l_try_hook("/proc/console_debug", dm_debug);
	l_try_hook("/proc/console_error", dm_error);
	l_try_hook("/proc/console_fatal", dm_fatal);
}

void Logging::LogProvider::Log(std::string message, LogLevel level) {
	Logging::Log(prov, message, level);
}

void Logging::LogProvider::Debug(std::string message) {
	Log(message, LOGLEVEL_DEBUG);
}

void Logging::LogProvider::Warning(std::string message) {
	Log(message, LOGLEVEL_WARNING);
}

void Logging::LogProvider::Error(std::string message) {
	Log(message, LOGLEVEL_ERROR);
}

void Logging::LogProvider::Fatal(std::string message) {
	Log(message, LOGLEVEL_FATAL);
}