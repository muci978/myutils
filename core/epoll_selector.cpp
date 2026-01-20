#include "epoll_selector.h"
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "logger.h"
#include "socketbase.h"
#include "process.h"

EpollSelector::EpollSelector(EventCallback r, EventCallback w, EventCallback e, int maxEvents, int timeout)
    : readHandler_(r),
      writeHandler_(w),
      errHandler_(e),
      epollFd_(-1),
      timeout_(timeout),
      maxEvents_(maxEvents),
      events_(maxEvents)
{
}

EpollSelector::~EpollSelector()
{
    if (-1 != epollFd_)
    {
        close(epollFd_);
        epollFd_ = -1;
    }
}

bool EpollSelector::Init()
{
    if (-1 != epollFd_)
    {
        error("epoll has been call Init, can not call again");
        return false;
    }
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (-1 == epollFd_)
    {
        error("epoll create error, err: {}", strerror(errno));
        return false;
    }

    return true;
}

bool EpollSelector::AddEvent(SocketBase *s, int event)
{
    if (-1 == epollFd_)
    {
        error("epoll has not call Init");
        return false;
    }
    if (nullptr == s || -1 == s->socket)
    {
        error("socket is nullptr");
        return false;
    }
    struct epoll_event ev;
    ev.events = event;
    ev.data.ptr = s;
    if (0 > epoll_ctl(epollFd_, EPOLL_CTL_ADD, s->socket, &ev))
    {
        error("epoll_ctl add error, err: {}", strerror(errno));
        return false;
    }

    return true;
}

bool EpollSelector::ModifyEvent(SocketBase *s, int event)
{
    if (-1 == epollFd_)
    {
        error("epoll has not call Init");
        return false;
    }
    if (nullptr == s || -1 == s->socket)
    {
        error("socket is nullptr");
        return false;
    }
    struct epoll_event ev;
    ev.events = event;
    ev.data.ptr = s;
    if (0 > epoll_ctl(epollFd_, EPOLL_CTL_MOD, s->socket, &ev))
    {
        error("epoll_ctl modify error, err: {}", strerror(errno));
        return false;
    }

    return true;
}

bool EpollSelector::RemoveEvent(SocketBase *s)
{
    if (-1 == epollFd_)
    {
        error("epoll has not call Init");
        return false;
    }
    if (nullptr == s)
    {
        warn("socket is nullptr");
        return false;
    }
    if (0 > epoll_ctl(epollFd_, EPOLL_CTL_DEL, s->socket, nullptr))
    {
        error("epoll_ctl del error, err: {}", strerror(errno));
        return false;
    }

    return true;
}

void EpollSelector::Work()
{
    while (!ProcessManager::GetInstance().IsPrepareExit())
    {
        int nfds = epoll_wait(epollFd_, events_.data(), maxEvents_, timeout_);
        if (nfds < 0)
        {
            error("epoll wait error, err: {}", strerror(errno));
            continue;
        }
        for (int i = 0; i < nfds; ++i)
        {
            SocketBase *socket = static_cast<SocketBase *>(events_[i].data.ptr);
            if (nullptr == socket)
            {
                continue;
            }
            if (events_[i].events & EPOLLERR)
            {
                errHandler_(this, socket);
                // TODO:将删除迁移到SocketBase的析构函数中？
                RemoveEvent(socket);
                continue;
            }
            if (events_[i].events & EPOLLIN)
            {
                readHandler_(this, socket);
            }
            if (events_[i].events & EPOLLOUT)
            {
                writeHandler_(this, socket);
            }
        }
    }
}