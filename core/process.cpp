#include "process.h"
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdexcept>
#include "logger.h"
#include "config.h"
#include "configinfo.h"

bool ProcessManager::prepareExit_ = false;

ProcessManager::ProcessManager()
{
}

bool ProcessManager::InitDaemon() const
{
    if (!ConfigManager::GetInstance().GetInfo()->daemonMode)
    {
        return false;
    }

    int pid = fork();
    if (0 > pid)
    {
        throw std::runtime_error(std::string("fork failed, err: ") + strerror(errno));
    }
    else if (0 < pid)
    {
        exit(EXIT_SUCCESS);
    }

    if (0 > setsid())
    {
        throw std::runtime_error(std::string("setsid failed, err: ") + strerror(errno));
    }

    umask(0);

    if (0 > chdir("/"))
    {
        throw std::runtime_error(std::string("chdir failed, err: ") + strerror(errno));
    }

    int fd = open("/dev/null", O_RDWR);
    if (0 > fd)
    {
        throw std::runtime_error(std::string("open /dev/null failed, err: ") + strerror(errno));
    }

    if (0 > dup2(fd, STDIN_FILENO))
    {
        throw std::runtime_error(std::string("dup2 failed, err: ") + strerror(errno));
    }
    if (0 > dup2(fd, STDOUT_FILENO))
    {
        throw std::runtime_error(std::string("dup2 failed, err: ") + strerror(errno));
    }
    if (0 > dup2(fd, STDERR_FILENO))
    {
        throw std::runtime_error(std::string("dup2 failed, err: ") + strerror(errno));
    }
    if (0 > close(fd))
    {
        throw std::runtime_error(std::string("close /dev/null failed, err: ") + strerror(errno));
    }

    return true;
}

bool ProcessManager::InitSignal() const
{
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    auto configInfo = ConfigManager::GetInstance().GetInfo();
    if (configInfo->daemonMode)
    {
        // 配置文件热加载依赖于console而不依赖信号
        act.sa_handler = SIG_IGN;
        if (0 > sigaction(SIGHUP, &act, nullptr))
        {
            error("set SIGHUP handler failed, err: {}", strerror(errno));
            return false;
        }
    }
    else
    {
        act.sa_handler = ExitHandler;
        if (0 > sigaction(SIGHUP, &act, nullptr))
        {
            error("set SIGHUP handler failed, err: {}", strerror(errno));
            return false;
        }
    }

    act.sa_handler = ExitHandler;
    if (0 > sigaction(SIGINT, &act, nullptr))
    {
        error("set SIGINT handler failed, err: {}", strerror(errno));
        return false;
    }
    if (0 > sigaction(SIGTERM, &act, nullptr))
    {
        error("set SIGTERM handler failed, err: {}", strerror(errno));
        return false;
    }
    if (0 > sigaction(SIGQUIT, &act, nullptr))
    {
        error("set SIGQUIT handler failed, err: {}", strerror(errno));
        return false;
    }

    act.sa_handler = SIG_IGN;
    if (0 > sigaction(SIGPIPE, &act, nullptr))
    {
        error("set SIGPIPE handler failed, err: {}", strerror(errno));
        return false;
    }
    if (0 > sigaction(SIGCHLD, &act, nullptr))
    {
        error("set SIGCHLD handler failed, err: {}", strerror(errno));
        return false;
    }

    // 发生coredump时先刷新日志，防止日志丢失
    act.sa_handler = [](int sig)
    {
        Logger::GetInstance().logger_->flush();
        struct sigaction act;
        act.sa_flags = 0;
        sigemptyset(&act.sa_mask);
        act.sa_handler = SIG_DFL;
        sigaction(sig, &act, nullptr);
        // 等待1秒确保日志刷新完成
        sleep(1);
        raise(SIGSEGV);
    };
    if (0 > sigaction(SIGSEGV, &act, nullptr))
    {
        error("set SIGSEGV handler failed, err: {}", strerror(errno));
        return false;
    }

    return true;
}

bool ProcessManager::InitCoreDump() const
{
    auto configInfo = ConfigManager::GetInstance().GetInfo();
    int coreLimitCur = configInfo->coreLimitCur;
    int coreLimitMax = configInfo->coreLimitMax;

    struct rlimit coreLimit;
    if (0 > getrlimit(RLIMIT_CORE, &coreLimit))
    {
        error("getrlimit failed, err: {}", strerror(errno));
        return false;
    }
    else
    {
        info("default rlimit: [max: {}, cur: {}]", coreLimit.rlim_max, coreLimit.rlim_cur);
    }

    coreLimit.rlim_max = coreLimitMax;
    coreLimit.rlim_cur = coreLimitCur;

    if (0 > setrlimit(RLIMIT_CORE, &coreLimit))
    {
        error("setrlimit failed, err: {}", strerror(errno));
        return false;
    }
    else
    {
        info("update rlimit: [max: {}, cur: {}]", coreLimit.rlim_max, coreLimit.rlim_cur);
    }

    return true;
}

bool ProcessManager::Init()
{
    info("process manager init");

    if (InitSignal())
    {
        info("init sig handlers successful");
    }
    else
    {
        error("init sig handlers failed");
        return false;
    }

    if (InitCoreDump())
    {
        info("init coredump file size successful");
    }
    else
    {
        error("init coredump file size failed");
        return false;
    }

    return true;
}

bool ProcessManager::Start()
{
    return true;
}
bool ProcessManager::Stop()
{
    return true;
}