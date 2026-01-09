#ifndef EPOLL_SELECTOR_H
#define EPOLL_SELECTOR_H

class EpollSelector
{
public:
    EpollSelector() = default;
    ~EpollSelector() = default;

public:
    void Init();
    void Start(int timeout);

private:
    int epollFd_;
};

#endif // EPOLL_SELECTOR_H
