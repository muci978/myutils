#ifndef CONFIGINFO_H
#define CONFIGINFO_H

#include <sstream>
#include <string>

struct ConfigInfo
{
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
    int coreLimitCur;
    int coreLimitMax;

    // 线程池相关
    bool useThreadPool;

    std::ostringstream configFile_;
    void Load(const std::string &configPath);
    void dumpConfig(std::ostream &os) const;
    std::string GetConfig() const;
};

#endif // CONFIGINFO_H