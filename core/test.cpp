#include "config.h"
#include "logger.h"

int main()
{
    auto &c = ConfigManager::GetInstance();
    c.Init("config.yml");
    auto &logger = Logger::GetInstance();
    logger.Init();
    warn("test1");
    info("test2");
    debug("test3");
    error("test4");
}