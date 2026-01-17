#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <iostream>
#include "singleton.h"

struct ConfigPimpl;

// TODO:通过控制台设置热加载
// TODO:控制台打印配置
class ConfigManager : public Singleton<ConfigManager>
{
    friend class Singleton<ConfigManager>;

public:
    void Init(const std::string &configPath);
    void DumpConfig(std::ostream &os) const;
    std::string GetConfig() const;
    const ConfigPimpl *const GetInfo() const
    {
        return info_;
    }

private:
    explicit ConfigManager();
    ~ConfigManager();

    void Load(const std::string& configPath) const;

private:
    ConfigPimpl *info_;
};

#endif // CONFIG_H