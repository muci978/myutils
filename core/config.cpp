#include "config.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

void ConfigManager::Init(const std::string &configPath)
{
    configPath_ = configPath;
    // 配置文件加载失败通过异常退出
    Load();
}

void ConfigManager::Load()
{
    YAML::Node root = YAML::LoadFile(configPath_);

    if (root["log"])
    {
        YAML::Node logNode = root["log"];
        logPath = logNode["logPath"].as<std::string>();
        logName = logNode["logName"].as<std::string>();
        logLevel = logNode["logLevel"].as<int>();
        logFileCount = logNode["logFileCount"].as<int>();
        rotateBySize = logNode["rotateBySize"].as<int>();
        rotateByDate = logNode["rotateByDate"].as<bool>();
        asyncMode = logNode["asyncMode"].as<bool>();
        asyncQueueSize = logNode["asyncQueueSize"].as<int>();
        asyncThreadSize = logNode["asyncThreadSize"].as<int>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'log' node");
    }

    if (root["reactor"])
    {
        YAML::Node reactorNode = root["reactor"];
        masterMaxEvent = reactorNode["masterMaxEvent"].as<int>();
        masterTimeout = reactorNode["masterTimeout"].as<int>();
        slaveMaxEvent = reactorNode["slaveMaxEvent"].as<int>();
        slaveTimeout = reactorNode["slaveTimeout"].as<int>();
        useSlave = reactorNode["useSlave"].as<bool>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'reactor' node");
    }

    if (root["console"])
    {
        YAML::Node consoleNode = root["console"];
        consoleListenIp = consoleNode["consoleListenIp"].as<std::string>();
        consoleListenPort = consoleNode["consoleListenPort"].as<int>();
        consoleTimeout = consoleNode["consoleTimeout"].as<int>();
        useConsole = consoleNode["useConsole"].as<bool>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'console' node");
    }

    if (root["process"])
    {
        YAML::Node processNode = root["process"];
        daemonMode = processNode["daemonMode"].as<bool>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'process' node");
    }

    if (root["threadpool"])
    {
        YAML::Node threadNode = root["threadpool"];
        useThreadPool = threadNode["useThreadPool"].as<bool>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'threadpool' node");
    }
}

void ConfigManager::dumpConfig(std::ostream &os) const
{
    os << std::boolalpha << std::left;

    os << "[log]" << '\n'
       << std::setw(20) << "logPath: " << logPath << '\n'
       << std::setw(20) << "logName: " << logName << '\n'
       << std::setw(20) << "logLevel: " << logLevel << '\n'
       << std::setw(20) << "logFileCount: " << logFileCount << '\n'
       << std::setw(20) << "rotateBySize: " << rotateBySize << '\n'
       << std::setw(20) << "rotateByDate: " << rotateByDate << '\n'
       << std::setw(20) << "asyncMode: " << asyncMode << '\n'
       << std::setw(20) << "asyncQueueSize: " << asyncQueueSize << '\n'
       << std::setw(20) << "asyncThreadSize: " << asyncThreadSize << '\n';
    os << '\n';

    os << "[reactor]" << '\n'
       << std::setw(20) << "masterMaxEvent: " << masterMaxEvent << '\n'
       << std::setw(20) << "masterTimeout: " << masterTimeout << '\n'
       << std::setw(20) << "slaveMaxEvnet: " << slaveMaxEvent << '\n'
       << std::setw(20) << "slaveTimeout: " << slaveTimeout << '\n'
       << std::setw(20) << "useSlave: " << useSlave << '\n';
    os << '\n';

    os << "[console]" << '\n'
       << std::setw(20) << "consoleListenIp: " << consoleListenIp << '\n'
       << std::setw(20) << "consoleListenPort: " << consoleListenPort << '\n'
       << std::setw(20) << "consoleTimeout: " << consoleTimeout << '\n'
       << std::setw(20) << "consoleMaxCount: " << consoleMaxCount << '\n'
       << std::setw(20) << "useConsole: " << useConsole << '\n';
    os << '\n';

    os << "[process]" << '\n'
       << std::setw(20) << "daemonMode: " << daemonMode << '\n';
    os << '\n';

    os << "[threadpool]" << '\n'
       << std::setw(20) << "usethreadpool: " << useThreadPool << '\n';
    os << '\n';

    os << std::noboolalpha << std::setw(0) << std::right;
}