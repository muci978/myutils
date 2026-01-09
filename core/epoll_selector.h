#ifndef EPOLL_SELECTOR_H
#define EPOLL_SELECTOR_H

class EpollSelector
{
public:
    EpollSelector() = default;
    ~EpollSelector() = default;

public:
    bool Init();
    bool Start(int timeout);

private:
    int epollFd_;
};

#endif // EPOLL_SELECTOR_H
