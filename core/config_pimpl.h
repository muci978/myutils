#ifndef CONFIG_PIMPL_H
#define CONFIG_PIMPL_H

#include <sstream>
#include <string>
#include <vector>

struct ConfigPimpl
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
    int reactorCount;
    int reactorMaxEvent;
    int reactorTimeout;
    std::vector<std::string> whiteList;
    std::vector<std::string> blackList;
    int maxConnection;
    int udpListenPort;
    int tcpListenPort;
    std::string udpListenIp;
    std::string tcpListenIp;
    int heartbeatInterval;
    int heartbeatTimeout;
    int heartbeatCheckInterval;

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

    void Init();
    void Load(const std::string &configPath);
    void dumpConfig(std::ostream &os) const;
    std::string GetConfig() const;

    std::ostringstream configFile_;
};

#endif // CONFIGINFO_H