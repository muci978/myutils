#ifndef REACTOR_PIMPL_H
#define REACTOR_PIMPL_H

#include <vector>
#include <string>
#include <memory>

class ThreadPool;
class EpollSelector;
class SocketBase;

class ReactorPimpl
{
public:
    bool Init();
    bool Start();
    bool Stop();

private:
    static void MasterReadHandler(EpollSelector *selector, SocketBase *s);
    static void MasterWriteHandler(EpollSelector *selector, SocketBase *s);
    static void MasterErrorHandler(EpollSelector *selector, SocketBase *s);
    static void SlaveReadHandler(EpollSelector *selector, SocketBase *s);
    static void SlaveWriteHandler(EpollSelector *selector, SocketBase *s);
    static void SlaveErrorHandler(EpollSelector *selector, SocketBase *s);

public:
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

public:
    std::unique_ptr<ThreadPool> slaveThreads_;
    std::shared_ptr<EpollSelector> master_;
    std::vector<std::shared_ptr<EpollSelector>> slaves_;
};

#endif // REACTOR_PIMPL_H