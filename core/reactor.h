#ifndef REACTOR_H
#define REACTOR_H

#include <vector>
#include <string>
#include "manager.h"
#include "singleton.h"

class ThreadPool;
class ReactorPimpl;

class ReactorManager : public Manager, public Singleton<ReactorManager>
{
public:
    virtual bool Init() override;
    virtual bool Start() override;
    virtual bool Stop() override;

private:
    explicit ReactorManager();
    ~ReactorManager();

    ReactorPimpl* base_;
};

#endif // REACTOR_H