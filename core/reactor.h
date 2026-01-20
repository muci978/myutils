#ifndef REACTOR_H
#define REACTOR_H

#include <vector>
#include <string>
#include "manager.h"

class ThreadPool;

class ReactorManager : public Manager
{
public:
    explicit ReactorManager();
    ~ReactorManager();

public:
    virtual bool Init() override;
    virtual bool Start() override;
    virtual bool Stop() override;

private:
    static void WorkerThread(ReactorManager *manager);

    ThreadPool *reactors_;
    int reactorCount_;
    int reactorMaxEvent_;
    int reactorTimeout_;
    std::vector<std::string> whiteList_;
    std::vector<std::string> blackList_;
    int maxConnection_;
    int udpListenPort_;
    int tcpListenPort_;
    std::string udpListenIp_;
    std::string tcpListenIp_;
    int heartbeatInterval_;
    int heartbeatTimeout_;
    int heartbeatCheckInterval_;
};

#endif // REACTOR_H