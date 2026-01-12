#include "epoll_selector.h"

#include <sys/epoll.h>
#include <error.h>
#include "logger.h"
#include "process.h"

EpollSelector::EpollSelector()
    : epollFd_(-1),
      configInfo_(nullptr)
{
}

EpollSelector::~EpollSelector()
{
    Stop();
}

bool EpollSelector::Init()
{        
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (0 > epollFd_)
    {
        error("epoll_create1 failed, err: {}", strerror(errno));
        return false;
    }

    return true;
}

bool EpollSelector::Start()
{
    for (;;)
    {
        if(ProcessManager::GetInstance().IsPrepareExit())
        {
            break;
        }
    }
    return true;
}

bool EpollSelector::Stop()
{
    if (0 < epollFd_)
    {
        close(epollFd_);
        epollFd_ = -1;
    }

    return true;
}