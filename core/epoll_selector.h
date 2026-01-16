#ifndef EPOLL_SELECTOR_H
#define EPOLL_SELECTOR_H

#include "manager.h"

class ConfigPimpl;

class EpollSelector : public Manager
{
public:
    EpollSelector();
    ~EpollSelector();

public:
    virtual bool Init() override;
    virtual bool Start() override;
    virtual bool Stop() override;

private:
    int epollFd_;
    ConfigPimpl* configInfo_;
};

#endif // EPOLL_SELECTOR_H
