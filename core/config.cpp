#include "config.h"
#include "config_pimpl.h"
#include <fstream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

void ConfigPimpl::Load(const std::string &configPath)
{
    auto ifs = std::ifstream(configPath);
    if(ifs.is_open() == false)
    {
        throw std::runtime_error("failed to open the configuration file");
    }
    configFile_ << ifs.rdbuf();
    ifs.close();

    YAML::Node root = YAML::LoadFile(configPath);

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
        reactorCount = reactorNode["reactorCount"].as<int>();
        reactorMaxEvent = reactorNode["reactorMaxEvent"].as<int>();
        reactorTimeout = reactorNode["reactorTimeout"].as<int>();
        whiteList = reactorNode["whiteList"].as<std::vector<std::string>>();
        blackList = reactorNode["blackList"].as<std::vector<std::string>>();
        maxConnection = reactorNode["maxConnection"].as<int>();
        udpListenPort = reactorNode["udpListenPort"].as<int>();
        tcpListenPort = reactorNode["tcpListenPort"].as<int>();
        udpListenIp = reactorNode["udpListenIp"].as<std::string>();
        tcpListenIp = reactorNode["tcpListenIp"].as<std::string>();
        heartbeatInterval = reactorNode["heartbeatInterval"].as<int>();
        heartbeatTimeout = reactorNode["heartbeatTimeout"].as<int>();
        heartbeatCheckInterval = reactorNode["heartbeatCheckInterval"].as<int>();
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
        coreLimitCur = processNode["coreLimitCur"].as<int>();
        coreLimitMax = processNode["coreLimitMax"].as<int>();
    }
    else
    {
        throw std::runtime_error("the configuration file is missing the 'process' node");
    }
}

void ConfigPimpl::dumpConfig(std::ostream &os) const
{
    os << configFile_.str();
}

std::string ConfigPimpl::GetConfig() const
{
    return configFile_.str();
}

ConfigManager::ConfigManager()
    : info_(new ConfigPimpl)
{
}

ConfigManager::~ConfigManager()
{
    if (info_)
    {
        delete info_;
        info_ = nullptr;
    }
}

void ConfigManager::Init(const std::string &configPath)
{
    // 配置文件加载失败通过异常退出
    Load(configPath);
    DumpConfig(std::cout);
}

void ConfigManager::Load(const std::string& configPath) const
{
    info_->Load(configPath);
}

void ConfigManager::DumpConfig(std::ostream &os) const
{
    info_->dumpConfig(os);
}

std::string ConfigManager::GetConfig() const
{
    return info_->GetConfig();
}