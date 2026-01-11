#include "logger.h"
#include <sstream>
#include <memory>
#include "config.h"
#include "configinfo.h"
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>

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

void LoggerConfig::Init()
{
    auto configInfo = ConfigManager::GetInstance().GetInfo();

    threadMode = (configInfo->useThreadPool | configInfo->useSlave) ? ThreadMode::Multi : ThreadMode::Single;
    outputTarget = configInfo->daemonMode ? OutputTarget::File : OutputTarget::Console;
    if (!configInfo->rotateByDate && 0 == configInfo->rotateBySize)
    {
        rotationPolicy = RotationPolicy::None;
    }
    else if (configInfo->rotateByDate && 0 == configInfo->rotateBySize)
    {
        rotationPolicy = RotationPolicy::ByDate;
    }
    else if (!configInfo->rotateByDate && configInfo->rotateBySize > 0)
    {
        rotationPolicy = RotationPolicy::BySize;
    }
    else
    {
        rotationPolicy = RotationPolicy::BySizeAndDate;
    }
    logPath = configInfo->logPath;
    logName = configInfo->logName;
    logFileCount = configInfo->logFileCount;
    logFileSize = configInfo->rotateBySize;
    switch (configInfo->logLevel)
    {
    case 0:
        logLevel = spdlog::level::level_enum::debug;
        break;
    case 1:
        logLevel = spdlog::level::level_enum::info;
        break;
    case 2:
        logLevel = spdlog::level::level_enum::warn;
        break;
    case 3:
        logLevel = spdlog::level::level_enum::err;
        break;
    default:
        logLevel = spdlog::level::level_enum::info;
        break;
    }
    asyncMode = configInfo->asyncMode;
    asyncQueueSize = configInfo->asyncQueueSize;
    asyncThreadSize = configInfo->asyncThreadSize;
}

std::ostringstream LoggerFactory::loggerMode_;

std::shared_ptr<spdlog::logger> LoggerFactory::CreateLogger(const LoggerConfig *config)
{
    std::vector<spdlog::sink_ptr> sinks;

    // 根据输出目标选择sink
    if (config->outputTarget == OutputTarget::Console)
    {
        sinks.emplace_back(CreateConsoleSink(config));
    }
    else
    {
        auto file_sink = CreateFileSink(config);
        if (nullptr != file_sink)
        {
            sinks.push_back(file_sink);
        }
    }

    // 创建logger
    std::shared_ptr<spdlog::logger> logger;
    loggerMode_ << "async: " << config->asyncMode << ", ";

    if (config->asyncMode)
    {
        logger = CreateAsyncLogger(config->loggerName, sinks, config);
    }
    else
    {
        logger = std::make_shared<spdlog::logger>(config->loggerName, sinks.begin(), sinks.end());
    }

    // 配置logger
    logger->set_level(config->logLevel);
    logger->flush_on(spdlog::level::level_enum::err);
    logger->set_pattern(GetLogPattern(config));
    spdlog::flush_every(std::chrono::seconds(1));

    loggerMode_ << "level: " << spdlog::level::to_short_c_str(logger->level());

    // 注册到全局注册表
    spdlog::register_logger(logger);

    return logger;
}

spdlog::sink_ptr LoggerFactory::CreateConsoleSink(const LoggerConfig *config)
{
    if (config->threadMode == ThreadMode::Multi)
    {
        loggerMode_ << "sink: stdout_color_sink_mt, ";
        return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    else
    {
        loggerMode_ << "sink: stdout_color_sink_st, ";
        return std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    }
}

spdlog::sink_ptr LoggerFactory::CreateFileSink(const LoggerConfig *config)
{
    switch (config->rotationPolicy)
    {
    case RotationPolicy::None:
        return CreateBasicFileSink(config);
    case RotationPolicy::BySize:
        return CreateRotatingSink(config);
    case RotationPolicy::ByDate:
        return CreateDailySink(config);
    case RotationPolicy::BySizeAndDate:
        return CreateSizeAndDateSink(config);
    default:
        return CreateBasicFileSink(config);
    }
}

spdlog::sink_ptr LoggerFactory::CreateBasicFileSink(const LoggerConfig *config)
{
    std::string fullPath = config->logPath + "/" + config->logName;

    if (config->threadMode == ThreadMode::Multi)
    {
        loggerMode_ << "sink: basic_file_sink_mt, ";
        return std::make_shared<spdlog::sinks::basic_file_sink_mt>(fullPath, true);
    }
    else
    {
        loggerMode_ << "sink: basic_file_sink_st, ";
        return std::make_shared<spdlog::sinks::basic_file_sink_st>(fullPath, true);
    }
}
spdlog::sink_ptr LoggerFactory::CreateRotatingSink(const LoggerConfig *config)
{
    std::string fullPath = config->logPath + "/" + config->logName;

    if (config->threadMode == ThreadMode::Multi)
    {
        loggerMode_ << "sink: rotating_file_sink_mt, ";
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fullPath,
            config->logFileSize * 1024 * 1024,
            config->logFileCount);
    }
    else
    {
        loggerMode_ << "sink: rotating_file_sink_st, ";
        return std::make_shared<spdlog::sinks::rotating_file_sink_st>(
            fullPath,
            config->logFileSize * 1024 * 1024,
            config->logFileCount);
    }
}

spdlog::sink_ptr LoggerFactory::CreateDailySink(const LoggerConfig *config)
{
    std::string fullPath = config->logPath + "/" + config->logName;

    if (config->threadMode == ThreadMode::Multi)
    {
        loggerMode_ << "sink: daily_file_sink_mt, ";
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            fullPath,
            0,     // 0点
            0,     // 0分
            false, // truncate
            static_cast<uint16_t>(config->logFileCount));
    }
    else
    {
        loggerMode_ << "sink: daily_file_sink_st, ";
        return std::make_shared<spdlog::sinks::daily_file_sink_st>(
            fullPath,
            0,     // 0点
            0,     // 0分
            false, // truncate
            static_cast<uint16_t>(config->logFileCount));
    }
}

std::shared_ptr<spdlog::logger> LoggerFactory::CreateAsyncLogger(const std::string &name, const std::vector<spdlog::sink_ptr> &sinks, const LoggerConfig *config)
{
    // 初始化线程池（全局）
    spdlog::init_thread_pool(config->asyncQueueSize, config->asyncThreadSize);
    loggerMode_ << "queSize: " << config->asyncQueueSize << ", ";
    loggerMode_ << "thdSize: " << config->asyncThreadSize << ", ";

    // 创建异步logger
    return std::make_shared<spdlog::async_logger>(
        name,
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
}

spdlog::sink_ptr LoggerFactory::CreateSizeAndDateSink(const LoggerConfig *config)
{
    // 需要创建两个sink同时工作
    // 暂时先默认使用大小轮转
    return CreateRotatingSink(config);
}

std::string LoggerFactory::GetLogPattern(const LoggerConfig *config)
{
    std::ostringstream oss;
    oss << "[%Y-%m-%d %H:%M:%S] ";
    oss << "[%^%l%$] ";
    if (config->threadMode == ThreadMode::Multi)
    {
        oss << "[tid:%t] ";
    }
    oss << "[%s:%#] %v";

    loggerMode_ << "pattern: " << oss.str() << ", ";
    return oss.str();
}

Logger::Logger() : logger_(nullptr), config_(new LoggerConfig) {}

Logger::~Logger()
{
    Stop();
}

void Logger::Init()
{
    config_->Init();
    // 日志系统初始化失败通过异常退出
    if(nullptr == logger_)
    {
        logger_ = LoggerFactory::CreateLogger(config_);
        info("logger initialized successfully");
        info(LoggerFactory::GetLoggerMode());
    }
}

void Logger::Stop()
{
    if (nullptr != logger_)
    {
        logger_->flush();
        logger_.reset();
        spdlog::drop_all();
        spdlog::shutdown();
    }

    if (nullptr != config_)
    {
        delete config_;
        config_ = nullptr;
    }
}