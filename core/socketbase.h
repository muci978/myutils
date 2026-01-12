#ifndef SOCKETBASE_H
#define SOCKETBASE_H

#include <stdint.h>
#include <string>

enum class SocketType
{
    UNKNOWN,
    UDP,
    TCP,
};

class SocketBase
{
public:
    SocketBase()
        : type_(SocketType::UNKNOWN),
          socket_(-1),
          localIp_(0),
          peerIp_(0),
          localPort_(0),
          peerPort_(0),
          ipv6Flag_(false),
          enable_(false) {}
    virtual ~SocketBase() = default;

    virtual void OnConnect() = 0;
    virtual void OnClose() = 0;
    virtual void OnError() = 0;
    virtual void OnRead() = 0;
    virtual void OnWrite() = 0;

public:
    SocketType type_;

    int socket_;
    uint32_t localIp_;
    uint32_t peerIp_;
    uint16_t localPort_;
    uint16_t peerPort_;

    bool ipv6Flag_;
    std::string localIpStr_;
    std::string peerIpStr_;

    bool enable_;
};

#endif // SOCKETBASE_H
