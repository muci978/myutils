#include "config.h"
#include "logger.h"
#include "controller.h"

const std::string configPath = "config.yml";

#include "manager.h"
class Test : public Manager
{
public:
    virtual bool Init() override
    {
        info("test init");
        return true;
    }
    virtual bool Start() override
    {
        info("test start");
        return true;
    }
    virtual bool Stop() override
    {
        info("test stop");
        return true;
    }
};

int main()
{
    auto &controller = Controller::GetInstance();
    auto pTest = new Test;
    controller.AddManager("TESTMANAGER", pTest);

    if (!controller.Start(configPath))
    {
        return -1;
    }
    controller.Stop();
    delete pTest;
    return 0;
}
