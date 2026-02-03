#ifndef EPOLL_SELECTOR_H
#define EPOLL_SELECTOR_H

#include <sys/epoll.h>
#include <functional>
#include <vector>
#include <memory>

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
    explicit EpollSelector(int maxEvents = EpollMaxEvents, int timeout = EpollTimeout);
    ~EpollSelector();

    bool AddEvent(SocketBase *s, int event);
    bool ModifyEvent(SocketBase *s, int event);
    bool RemoveEvent(SocketBase *s);

    void Work(std::function<bool()> stop);
    void SetReadHandler(EventCallback cb)
    {
        readHandler_ = cb;
    }
    void SetWriteHandler(EventCallback cb)
    {
        writeHandler_ = cb;
    }
    void SetErrHandler(EventCallback cb)
    {
        errHandler_ = cb;
    }

private:
    int epollFd_;
    const int timeout_;
    const int maxEvents_;
    std::vector<struct epoll_event> events_;

    EventCallback readHandler_;
    EventCallback writeHandler_;
    EventCallback errHandler_;
};

#endif // EPOLL_SELECTOR_H