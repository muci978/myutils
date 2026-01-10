#include "config.h"
#include "configinfo.h"


ConfigManager::ConfigManager() : info_(new ConfigInfo)
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
    configPath_ = configPath;
    // 配置文件加载失败通过异常退出
    Load();
}

void ConfigManager::Load()
{
    info_->Load(configPath_);
}

void ConfigManager::dumpConfig(std::ostream &os) const
{
    info_->dumpConfig(os);
}