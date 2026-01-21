#pragma once
#include "../common/common.h"

enum class LogLevel { Info, Warn, Error };

constexpr const char* color(LogLevel l) {
    switch(l) {
        case LogLevel::Info:  return "\033[0m";
        case LogLevel::Warn:  return "\033[1;33m";
        case LogLevel::Error: return "\033[1;31m";
    }
    return "\033[0m";
}

class Logger {
    std::string FileName = "./logs/log.txt";
    bool logInConsole = true;
    bool logInFile = true;

public:
    ~Logger(void){}
    void Clear();
    void log(LogLevel lvl, const std::string& s);
};
static inline void log_impl(LogLevel lvl, const std::string& s) {
    Logger logger{};
    logger.log(lvl,s);
    logger.~Logger();
}

    template<typename... Args>
    static void LOG(const Args&... args) {
        std::ostringstream oss;
        ((oss << args), ...);
        log_impl(LogLevel::Info, oss.str());
    }
    template<typename... Args>
    static void WARN(const Args&... args) {
        std::ostringstream oss;
        ((oss << args), ...);
        log_impl(LogLevel::Warn, oss.str());
    }
    template<typename... Args>
    static void ERROR(const Args&... args) {
        std::ostringstream oss;
        ((oss << args), ...);
        log_impl(LogLevel::Error, oss.str());
    }
