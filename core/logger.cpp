#include "logger.h"
#include "config.h"
#include <sstream>
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>

void LoggerConfig::Init()
{
    auto &configManager = ConfigManager::GetInstance();

    threadMode = (configManager.useThreadPool | configManager.useSlave) ? ThreadMode::Multi : ThreadMode::Single;
    outputTarget = configManager.daemonMode ? OutputTarget::File : OutputTarget::Console;
    if (!configManager.rotateByDate && 0 == configManager.rotateBySize)
    {
        rotationPolicy = RotationPolicy::None;
    }
    else if (configManager.rotateByDate && 0 == configManager.rotateBySize)
    {
        rotationPolicy = RotationPolicy::ByDate;
    }
    else if (!configManager.rotateByDate && configManager.rotateBySize > 0)
    {
        rotationPolicy = RotationPolicy::BySize;
    }
    else
    {
        rotationPolicy = RotationPolicy::BySizeAndDate;
    }
    logPath = configManager.logPath;
    logName = configManager.logName;
    logFileCount = configManager.logFileCount;
    logFileSize = configManager.rotateBySize;
    switch (configManager.logLevel)
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
    asyncMode = configManager.asyncMode;
    asyncQueueSize = configManager.asyncQueueSize;
    asyncThreadSize = configManager.asyncThreadSize;
}

std::shared_ptr<spdlog::logger> LoggerFactory::CreateLogger(const LoggerConfig &config)
{
    std::vector<spdlog::sink_ptr> sinks;

    // 根据输出目标选择sink
    if (config.outputTarget == OutputTarget::Console)
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
    if (config.asyncMode)
    {
        logger = CreateAsyncLogger(config.loggerName, sinks, config);
    }
    else
    {
        logger = std::make_shared<spdlog::logger>(config.loggerName, sinks.begin(), sinks.end());
    }

    // 配置logger
    logger->set_level(config.logLevel);
    logger->flush_on(spdlog::level::level_enum::err);
    logger->set_pattern(GetLogPattern(config));
    spdlog::flush_every(std::chrono::seconds(5));

    // 注册到全局注册表
    spdlog::register_logger(logger);

    return logger;
}

spdlog::sink_ptr LoggerFactory::CreateConsoleSink(const LoggerConfig &config)
{
    if (config.threadMode == ThreadMode::Multi)
    {
        return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    else
    {
        return std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    }
}

spdlog::sink_ptr LoggerFactory::CreateFileSink(const LoggerConfig &config)
{
    switch (config.rotationPolicy)
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

spdlog::sink_ptr LoggerFactory::CreateBasicFileSink(const LoggerConfig &config)
{
    std::string fullPath = config.logPath + "/" + config.logName;

    if (config.threadMode == ThreadMode::Multi)
    {
        return std::make_shared<spdlog::sinks::basic_file_sink_mt>(fullPath, true);
    }
    else
    {
        return std::make_shared<spdlog::sinks::basic_file_sink_st>(fullPath, true);
    }
}
spdlog::sink_ptr LoggerFactory::CreateRotatingSink(const LoggerConfig &config)
{
    std::string fullPath = config.logPath + "/" + config.logName;

    if (config.threadMode == ThreadMode::Multi)
    {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fullPath,
            config.logFileSize * 1024 * 1024,
            config.logFileCount);
    }
    else
    {
        return std::make_shared<spdlog::sinks::rotating_file_sink_st>(
            fullPath,
            config.logFileSize * 1024 * 1024,
            config.logFileCount);
    }
}

spdlog::sink_ptr LoggerFactory::CreateDailySink(const LoggerConfig &config)
{
    std::string fullPath = config.logPath + "/" + config.logName;

    if (config.threadMode == ThreadMode::Multi)
    {
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            fullPath,
            0,     // 0点
            0,     // 0分
            false, // truncate
            static_cast<uint16_t>(config.logFileCount));
    }
    else
    {
        return std::make_shared<spdlog::sinks::daily_file_sink_st>(
            fullPath,
            0,     // 0点
            0,     // 0分
            false, // truncate
            static_cast<uint16_t>(config.logFileCount));
    }
}

std::shared_ptr<spdlog::logger> LoggerFactory::CreateAsyncLogger(const std::string &name, const std::vector<spdlog::sink_ptr> &sinks, const LoggerConfig &config)
{
    // 初始化线程池（全局）
    spdlog::init_thread_pool(config.asyncQueueSize, config.asyncThreadSize);

    // 创建异步logger
    return std::make_shared<spdlog::async_logger>(
        name,
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
}

spdlog::sink_ptr LoggerFactory::CreateSizeAndDateSink(const LoggerConfig &config)
{
    // 需要创建两个sink同时工作
    // 暂时先默认使用大小轮转
    return CreateRotatingSink(config);
}

std::string LoggerFactory::GetLogPattern(const LoggerConfig &config)
{
    std::ostringstream oss;
    oss << "[%Y-%m-%d %H:%M:%S] ";
    oss << "[%^%l%$] ";
    if (config.threadMode == ThreadMode::Multi)
    {
        oss << "[tid:%t] ";
    }
    oss << "[%s:%#] %v";
    return oss.str();
}

void Logger::Init()
{
    config_.Init();
    // 日志系统初始化失败通过异常退出
    logger_ = LoggerFactory::CreateLogger(config_);
}
