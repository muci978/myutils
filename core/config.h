#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <iostream>
#include "singleton.h"

// TODO:通过控制台设置热加载
// TODO:控制台打印配置
class ConfigManager : public Singleton<ConfigManager>
{
    friend class Singleton<ConfigManager>;

public:
    void Init(const std::string &configPath);
    void dumpConfig(std::ostream &os) const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    void Load();

private:
    std::string configPath_;

public:
    // 日志相关
    std::string logPath;
    std::string logName;
    int logLevel;
    int logFileCount;
    int rotateBySize;
    bool rotateByDate;
    bool asyncMode;
    int asyncQueueSize;
    int asyncThreadSize;

    // reactor相关
    int masterMaxEvent;
    int masterTimeout;
    int slaveMaxEvent;
    int slaveTimeout;
    bool useSlave;

    // 控制台相关
    std::string consoleListenIp;
    int consoleListenPort;
    int consoleTimeout;
    int consoleMaxCount;
    bool useConsole;

    // 进程相关
    bool daemonMode;

    // 线程池相关
    bool useThreadPool;
};

#endif // CONFIG_H