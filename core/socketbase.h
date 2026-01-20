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
        : type(SocketType::UNKNOWN),
          socket(-1),
          localIp(0),
          peerIp(0),
          localPort(0),
          peerPort(0),
          ipv6Flag(false),
          enable_(false) {}
    virtual ~SocketBase() = default;

    virtual void OnConnect() = 0;
    virtual void OnClose() = 0;
    virtual void OnError() = 0;
    virtual void OnRead() = 0;
    virtual void OnWrite() = 0;

public:
    SocketType type;

    int socket;
    uint32_t localIp;
    uint32_t peerIp;
    uint16_t localPort;
    uint16_t peerPort;

    bool ipv6Flag;
    std::string localIpStr;
    std::string peerIpStr;

    bool enable_;
};

#endif // SOCKETBASE_H
