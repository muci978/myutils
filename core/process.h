#ifndef PROCESS_H_
#define PROCESS_H_

#include "singleton.h"
#include "manager.h"

class ProcessManager : public Singleton<ProcessManager>, public Manager
{
    friend class Singleton<ProcessManager>;

private:
    ProcessManager();
    ~ProcessManager() = default;

    static void ExitHandler(int sig)
    {
        prepareExit_ = true;
    }

    bool InitSignal() const;
    bool InitCoreDump() const;
    // TODO：文件描述符个数限制？
    // TODO：绑定main函数cpu？
    // TODO：获取设备信息？
    // TODO：设置进程title？

public:
    // 开启daemon返回true，否则返回false
    // 创建守护进程需要fork子进程，会导致spdlog异步模式下锁不可用，所以单独进行调用
    bool InitDaemon() const;

    virtual bool Init() override;
    virtual bool Start() override;
    virtual bool Stop() override;

    void SetPrepareExit(bool flag) { prepareExit_ = flag; }
    bool IsPrepareExit() { return prepareExit_; }

private:
    static bool prepareExit_;
};

#endif // PROCESS_H_