#include "reactor.h"
#include <stdexcept>
#include "config.h"
#include "config_pimpl.h"
#include "threadpool.h"
#include "logger.h"
#include "epoll_selector.h"
#include "reactor_pimpl.h"
#include "socketbase.h"

void ReactorPimpl::MasterReadHandler(EpollSelector *selector, SocketBase *s)
{
    if (nullptr == selector || nullptr == s)
    {
        error("invalid param");
        return;
    }

    if (SocketType::TCP == s->type)
    {
        
    }
}

bool ReactorPimpl::Init()
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

    slaveThreads_.reset(new ThreadPool("SlaveReactorPool", reactorCount_, reactorCount_));
    if (nullptr == slaveThreads_)
    {
        error("create slave thread pool failed, err: {}", std::strerror(errno));
        return false;
    }
    master_ = std::make_shared<EpollSelector>(reactorMaxEvent_, reactorTimeout_);
    if (nullptr == master_)
    {
        error("create master selector failed, err: {}", std::strerror(errno));
        return false;
    }
    slaves_.resize(reactorCount_);
    for (int i = 0; i < reactorCount_; ++i)
    {
        slaves_[i] = std::make_shared<EpollSelector>(reactorMaxEvent_, reactorTimeout_);
        if (nullptr == slaves_[i])
        {
            error("create slave selector failed, err: {}", std::strerror(errno));
            return false;
        }
    }

    return true;
}

ReactorManager::ReactorManager()
    : base_(new ReactorPimpl) {}

ReactorManager::~ReactorManager()
{
    Stop();
    delete base_;
    base_ = nullptr;
}

bool ReactorManager::Init()
{
    return base_->Init();
}

bool ReactorManager::Start()
{
    return base_->Start();
}

bool ReactorManager::Stop()
{
    return base_->Stop();
}