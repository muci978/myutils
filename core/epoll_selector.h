#ifndef EPOLL_SELECTOR_H
#define EPOLL_SELECTOR_H

#include <sys/epoll.h>
#include <functional>
#include <vector>

class SocketBase;

enum
{
    EpollMaxEvents = 1024,
    EpollTimeout = 100,
};

enum
{
    Read = EPOLLIN | EPOLLRDHUP,
    Write = EPOLLOUT,
    ReadWrite = Read | Write,
    ET = EPOLLET,
    Oneshot = EPOLLONESHOT,
};
class EpollSelector
{
    using EventCallback = std::function<void(EpollSelector *, SocketBase *)>;

public:
    EpollSelector(EventCallback r, EventCallback w, EventCallback e, int maxEvents = EpollMaxEvents, int timeout = EpollTimeout);
    ~EpollSelector();

    bool Init();

    bool AddEvent(SocketBase *s, int event);
    bool ModifyEvent(SocketBase *s, int event);
    bool RemoveEvent(SocketBase *s);

    void Work();

private:
    int epollFd_;
    int timeout_;
    int maxEvents_;
    std::vector<struct epoll_event> events_;

    EventCallback readHandler_;
    EventCallback writeHandler_;
    EventCallback errHandler_;
};

#endif // EPOLL_SELECTOR_H