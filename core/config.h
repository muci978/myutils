#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <iostream>
#include "singleton.h"

struct ConfigInfo;

// TODO:通过控制台设置热加载
// TODO:控制台打印配置
class ConfigManager : public Singleton<ConfigManager>
{
    friend class Singleton<ConfigManager>;

public:
    void Init(const std::string &configPath);
    void dumpConfig(std::ostream &os) const;
    const ConfigInfo * const GetInfo() const
    {
        return info_;
    }

private:
    ConfigManager();
    ~ConfigManager();

    void Load();

private:
    ConfigInfo *info_;
    std::string configPath_;
};

#endif // CONFIG_H