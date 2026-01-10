#include "config.h"
#include "logger.h"
#include <thread>

int main()
{
    auto &c = ConfigManager::GetInstance();
    c.Init("config.yml");
    auto &logger = Logger::GetInstance();
    logger.Init();
    logger.Close();
}