#include "controller.h"
#include "config.h"
#include "utils.h"
#include "logger.h"
#include "process.h"
#include "epoll_selector.h"

Controller::Controller()
    : config_(ConfigManager::GetInstance()),
      process_(ProcessManager::GetInstance()),
      logger_(Logger::GetInstance()),
      selector_(new EpollSelector) {}

bool Controller::Start(const std::string &configPath)
{
    config_.Init(configPath);

    systemManagers_.clear();
    systemManagers_.emplace_back("ProcessManager", &process_);

    // init all managers
    for (auto &manager : systemManagers_)
    {
        if (!manager.second->Init())
        {
            return false;
        }
        info("init manager: {}", manager.first);
    }
    for (auto &manager : customManagers_)
    {
        if (!manager.second->Init())
        {
            return false;
        }
        info("init manager: {}", manager.first);
    }

    // start all managers
    for (auto &manager : systemManagers_)
    {
        if (!manager.second->Start())
        {
            return false;
        }
        info("start manager: {}", manager.first);
    }
    for (auto &manager : customManagers_)
    {
        if (!manager.second->Start())
        {
            return false;
        }
        info("start manager: {}", manager.first);
    }

    if (!selector_->Init())
    {
        return false;
    }
    info("init epoll selector");

    info("start epoll selector");
    if (!selector_->Start())
    {
        return false;
    }

    return true;
}

bool Controller::Stop() const
{
    debug("stop epoll selector");
    selector_->Stop();

    auto rbegin = customManagers_.rbegin();
    auto rend = customManagers_.rend();
    while (rbegin != rend)
    {
        debug("stop manager: {}", rbegin->first);
        rbegin->second->Stop();
        ++rbegin;
    }

    rbegin = systemManagers_.rbegin();
    rend = systemManagers_.rend();
    while (rbegin != rend)
    {
        debug("stop manager: {}", rbegin->first);
        rbegin->second->Stop();
        ++rbegin;
    }

    info("stop all managers, exit");
    logger_.Stop();

    return true;
}

void Controller::AddManager(const std::string &name, Manager *manager)
{
    customManagers_.emplace_back(Trim(name), manager);
}

void Controller::RemoveManager(const std::string &name)
{
    auto it = customManagers_.begin();
    for (; it != customManagers_.end(); ++it)
    {
        if (it->first == Trim(name))
        {
            break;
        }
    }
    if (it != customManagers_.end())
    {
        customManagers_.erase(it);
    }
}