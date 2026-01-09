#ifndef CONSOLE_H
#define CONSOLE_H

#include <unistd.h>
#include <stdint.h>

class Console
{
public:
    void Init(uint16_t port);
    void Start() const;

private:
    uint16_t port_;
};

#endif // CONSOLE_H