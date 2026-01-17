#include "config.h"
#include "logger.h"
#include "controller.h"
#include "threadpool.h"
#include <chrono>
#include <exception>
#include <future>
#include <thread>

const std::string configPath = "config.yml";

#include "manager.h"

int TestTask(int a, int b)
{
    return a + b;
}

void TestTaskexc(int a, int b)
{
    throw std::runtime_error("Intentional Exception");
}

int TestSleepTask(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    return 1;
}

class TestManager : public Manager
{
public:
    TestManager() = default;
    ~TestManager() override = default;

    bool Init() override
    {
        return true;
    }

    bool Start() override
    {
        /**
         * case1:启动前插入任务
         * case2:运行中插入任务
         * case3:停止后插入任务
         * case4:优雅关闭时插入任务
         * case5:优雅关闭
         * case6:强制关闭
         * case7:任务执行异常
         * case8:返回值的任务
         * case9:不返回值的任务
         * case10:任务过多，队列满了
         * case11:多线程插入任务
         */
        ThreadPool tp("TestThreadPool", 10, 5);

        tp.Start();

        std::vector<std::future<int>> results;
        for (int i = 0; i < 10; ++i)
        {
            results.emplace_back(tp.AddTask(TestSleepTask, 1));
        }

        std::async(std::launch::async, [&]()
                   {
                       try
                       {
                           for (int i = 0; i < 100; ++i)
                           {
                               std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                               results.emplace_back(tp.AddTask(TestSleepTask, 1));
                           }
                       }
                       catch (const std::exception &e)
                       {
                           warn("AddTask after Stop: {}", e.what());
                       } });
        tp.Stop(true);

        for (auto &result : results)
        {
            try
            {
                info("result: {}", result.get());
            }
            catch (std::exception &e)
            {
                warn("result: {}", e.what());
            }
            catch (...)
            {
                warn("result: unknown error");
            }
        }

        return true;
    }

    bool Stop() override
    {
        return true;
    }
};

int main()
{
    auto &controller = Controller::GetInstance();

    controller.AddManager("TestManager", new TestManager());

    if (!controller.Start(configPath))
    {
        return -1;
    }
    controller.Stop();
    return 0;
}
