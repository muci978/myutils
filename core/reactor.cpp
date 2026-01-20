#include "reactor.h"
#include <stdexcept>
#include "config.h"
#include "config_pimpl.h"
#include "threadpool.h"
#include "logger.h"
#include "epoll_selector.h"

ReactorManager::ReactorManager() {}

ReactorManager::~ReactorManager()
{
    Stop();
}

bool ReactorManager::Init()
{
    auto configInfo = ConfigManager::GetInstance().GetInfo();
    reactorCount_ = configInfo->reactorCount;
    reactorMaxEvent_ = configInfo->reactorMaxEvent;
    reactorTimeout_ = configInfo->reactorTimeout;
    maxConnection_ = configInfo->maxConnection;
    udpListenPort_ = configInfo->udpListenPort;
    tcpListenPort_ = configInfo->tcpListenPort;
    udpListenIp_ = configInfo->udpListenIp;
    tcpListenIp_ = configInfo->tcpListenIp;
    heartbeatInterval_ = configInfo->heartbeatInterval;
    heartbeatTimeout_ = configInfo->heartbeatTimeout;
    heartbeatCheckInterval_ = configInfo->heartbeatCheckInterval;
    whiteList_ = configInfo->whiteList;
    blackList_ = configInfo->blackList;

    return true;
}

bool ReactorManager::Start()
{
    reactors_ = new ThreadPool("ReactorPool", reactorCount_, reactorCount_);

    if (!reactors_->Start())
    {
        return false;
    }
    for(int i = 0; i < reactorCount_; ++i)
    {
        try
        {
            auto ret = reactors_->AddTask(WorkerThread, this);
            ret.get();
        }
        catch (const std::exception &e)
        {
            error("{} throws exception: {}", "ReactorPool", e.what());
            Stop();
            return false;
        }
    }

    return true;
}

bool ReactorManager::Stop()
{
    if(nullptr != reactors_)
    {
        reactors_->Stop();
        delete reactors_;
        reactors_ = nullptr;
    }
    return true;
}

void ReactorManager::WorkerThread(ReactorManager *manager)
{
    EpollSelector selector;
    if(!selector.Init())
    {
        throw std::runtime_error("EpollSelector init failed");
    }
    if(!selector.Start())
    {
        throw std::runtime_error("EpollSelector start failed");
    }
    
}