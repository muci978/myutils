#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include "singleton.h"

enum LogLevel
{
    MIN_LEVEL = -1,
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    MAX_LEVEL = 4
};

// TODO：设置日志级别，例如：DEBUG, INFO, WARN, ERROR
// TODO：根据日志级别，决定是否进行过滤
// TODO：支持自定义日志格式，例如时间戳、文件名和行号等
// TODO：输出到日志文件，而非标准错误输出
class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;

public:
    void Log(LogLevel level, const std::string &msg) const
    {
        std::string levelStr;
        switch (level)
        {
        case DEBUG:
            levelStr = "[DEBUG] ";
            break;
        case INFO:
            levelStr = "[INFO] ";
            break;
        case WARN:
            levelStr = "[WARN] ";
            break;
        case ERROR:
            levelStr = "[ERROR] ";
            break;
        default:
            levelStr = "[UNKNOWN] ";
            break;
        }
        if (level > limit_)
        {
            std::cerr << levelStr << msg << std::endl;
        }
    }

private:
    Logger(LogLevel limit = MIN_LEVEL) : limit_(limit) {}
    ~Logger() {}

private:
    LogLevel limit_;
};

#endif // LOGGER_H