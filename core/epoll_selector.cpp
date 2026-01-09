#include "epoll_selector.h"

#include <sys/epoll.h>
#include <error.h>
#include "logger.h"

extern 

bool EpollSelector::Init()
{
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0)
    {
        throw std::runtime_error("epoll create failed, err: " + std::to_string(errno));
    }
}

bool EpollSelector::Start(int timeout)
{
}