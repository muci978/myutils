#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include "singleton.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

// #define trace(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::trace, __VA_ARGS__)
#define debug(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::debug, __VA_ARGS__)
#define info(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::info, __VA_ARGS__)
#define warn(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::warn, __VA_ARGS__)
#define error(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::err, __VA_ARGS__)
// #define critical(...) SPDLOG_LOGGER_CALL(Logger::GetInstance().logger_.get(), spdlog::level::critical, __VA_ARGS__)

enum class ThreadMode
{
    Single, // 单线程模式
    Multi   // 多线程模式
};

enum class OutputTarget
{
    Console,
    File
};

enum class RotationPolicy
{
    None,
    BySize,
    ByDate,
    BySizeAndDate
};

struct LoggerConfig
{
    void Init();

    ThreadMode threadMode;
    OutputTarget outputTarget;
    RotationPolicy rotationPolicy;
    std::string logPath;
    std::string logName;
    int logFileCount;
    int logFileSize;
    spdlog::level::level_enum logLevel;
    bool asyncMode;
    int asyncQueueSize;
    int asyncThreadSize;

    std::string loggerName = "AppLogger";
};

// TODO:console输出
class LoggerFactory
{
public:
    static std::shared_ptr<spdlog::logger> CreateLogger(const LoggerConfig &config);
    static std::string GetLoggerMode()
    {
        return loggerMode_.str();
    }

private:
    static spdlog::sink_ptr CreateConsoleSink(const LoggerConfig &config);
    static spdlog::sink_ptr CreateFileSink(const LoggerConfig &config);
    static spdlog::sink_ptr CreateBasicFileSink(const LoggerConfig &config);
    static spdlog::sink_ptr CreateRotatingSink(const LoggerConfig &config);
    static spdlog::sink_ptr CreateDailySink(const LoggerConfig &config);
    static spdlog::sink_ptr CreateSizeAndDateSink(const LoggerConfig &config);
    static std::shared_ptr<spdlog::logger> CreateAsyncLogger(const std::string &name, const std::vector<spdlog::sink_ptr> &sinks, const LoggerConfig &config);
    static std::string GetLogPattern(const LoggerConfig &config);

private:
    static std::ostringstream loggerMode_;
};

class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;

public:
    void Init();
    void Close();

private:
    Logger() : logger_(nullptr) {}
    ~Logger()
    {
        Close();
    }

public:
    std::shared_ptr<spdlog::logger> logger_;

private:
    LoggerConfig config_;
};

#endif // LOGGER_H