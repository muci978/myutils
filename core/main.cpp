#include "config.h"
#include "logger.h"
#include "controller.h"

const std::string configPath = "config.yml";

#include "manager.h"

int main()
{
    auto &controller = Controller::GetInstance();

    if (!controller.Start(configPath))
    {
        return -1;
    }
    controller.Stop();
    return 0;
}
