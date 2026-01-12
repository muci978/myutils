#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <utility>
#include <vector>
#include <string>
#include "singleton.h"

class Manager;
class ConfigManager;
class ProcessManager;
class EpollSelector;
class Logger;

class Controller : public Singleton<Controller>
{
    friend class Singleton<Controller>;

public:
    using ManagerQueue = std::vector<std::pair<std::string, Manager *>>;

    void AddManager(const std::string &name, Manager *manager);
    void RemoveManager(const std::string &name);
    bool Start(const std::string &configPath);
    bool Stop() const;

private:
    Controller();
    ~Controller() = default;

private:
    ManagerQueue systemManagers_;
    ManagerQueue customManagers_;

    ConfigManager &config_;
    ProcessManager &process_;
    Logger &logger_;
    EpollSelector *selector_;
};

#endif // CONTROLLER_H