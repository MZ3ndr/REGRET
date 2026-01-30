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
    #if DEBUG
        bool logInConsole = true;
    #else
        bool logInColsole = false;
    #endif
    bool logInFile = true;

public:
    ~Logger(void){}
    void Clear();
    void log(LogLevel lvl, const std::string& s);
};

inline std::underlying_type_t<REGRET::RG_Result> log_arg(REGRET::RG_Result r) {
    return static_cast<std::underlying_type_t<REGRET::RG_Result>>(r);
}

static inline void log_impl(LogLevel lvl, const std::string& s) {
    Logger logger{};
    logger.log(lvl,s);
}

template<typename T>
decltype(auto) log_arg(const T& v) {
    if constexpr (std::is_enum_v<T>) {
        return static_cast<std::underlying_type_t<T>>(v);
    } else {
        return v;
    }
}

template<typename... Args>
static void LOG(const Args&... args) {
    std::ostringstream oss;
    ((oss << log_arg(args)), ...);
    log_impl(LogLevel::Info, oss.str());
}
template<typename... Args>
static void WARN(const Args&... args) {
    std::ostringstream oss;
    ((oss << log_arg(args)), ...);
    log_impl(LogLevel::Warn, oss.str());
}
template<typename... Args>
static void ERROR(const Args&... args) {
    std::ostringstream oss;
    ((oss << log_arg(args)), ...);
    log_impl(LogLevel::Error, oss.str());
}


