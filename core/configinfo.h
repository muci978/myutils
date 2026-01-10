#ifndef CONFIGINFO_H
#define CONFIGINFO_H

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

    // 线程池相关
    bool useThreadPool;

    void Load(const std::string &configPath);
    void dumpConfig(std::ostream &os) const;
};

#endif // CONFIGINFO_H