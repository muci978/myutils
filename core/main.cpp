#include <memory>
#include "config.h"
#include "logger.h"
#include "process.h"

const std::string configPath = "config.yml";

auto &config = ConfigManager::GetInstance();
auto &logger = Logger::GetInstance();
auto &process = ProcessManager::GetInstance();

int main()
{
    bool ret;
    std::unique_ptr<int, void (*)(int *)> deleter(new int, [](int *)
                                                  {
        info("main exit");
        logger.Stop();
        process.Stop(); });

    config.Init(configPath);

    ret = process.InitDaemon();
    logger.Init();

    if (ret)
    {
        info("the process runs as a daemon");
    }

    if (!process.Init())
    {
        return -1;
    }

    if (!process.Start())
    {
        return -1;
    }

    int *trigger = nullptr;
    *trigger = 100;

    config.dumpConfig(std::cout);
    for (;;)
    {
        if (process.IsPrepareExit())
        {
            return 0;
        }
    }
}
