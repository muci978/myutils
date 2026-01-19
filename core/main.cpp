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

int TestTaskExp(int a, int b)
{
    throw std::runtime_error("Intentional Exception");
    return 0;
}

int TestSleepTask(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    return 1;
}

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
void case1()
{
    ThreadPool tp("case1", 1, 1);
    auto result = tp.AddTask(TestTask, 1, 2);
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
    tp.Start();
    tp.Stop();
}

void case2()
{
    ThreadPool tp("case2", 1, 1);
    tp.Start();
    auto result = tp.AddTask(TestTask, 1, 2);
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
    tp.Stop();
}

void case3()
{
    ThreadPool tp("case3", 1, 1);
    tp.Start();
    tp.Stop();
    auto result = tp.AddTask(TestTask, 1, 2);
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

void case4()
{
    ThreadPool tp("case4", 1, 1);
    tp.Start();
    tp.Stop(true);
    auto result = tp.AddTask(TestTask, 1, 2);
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

void case5()
{
    ThreadPool tp("case5", 100, 1);
    tp.Start();
    for (int i = 0; i < 100; ++i)
    {
        tp.AddTask(TestSleepTask, 1);
    }
    tp.Stop(true);
}

void case6()
{
    ThreadPool tp("case6", 100, 1);
    tp.Start();
    for (int i = 0; i < 100; ++i)
    {
        tp.AddTask(TestSleepTask, 1);
    }
    tp.Stop(false);
}

void case7()
{
    ThreadPool tp("case7", 100, 1);
    tp.Start();

    auto result = tp.AddTask(TestTaskExp, 1, 2);
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

    tp.Stop(true);
}

void case9()
{
    ThreadPool tp("case9", 100, 1);
    tp.Start();

    auto result = tp.AddTask([]
                             { std::this_thread::sleep_for(std::chrono::seconds(1)); });
    result.get();
    info("case9: sleep 1s");
}

void case10()
{
    ThreadPool tp("case10", 1, 1);
    std::vector<std::future<int>> results;
    tp.Start();
    for (int i = 0; i < 5; ++i)
    {
        results.emplace_back(tp.AddTask(TestSleepTask, 1));
    }
    tp.Stop();
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
}

void case11()
{
    auto now = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    ThreadPool tp("case11", 100, 1);
    tp.Start();
    auto f = std::thread([&]
                         {
                             std::this_thread::sleep_until(now);
                             std::vector<std::future<int>> results;
                             for (int i = 0; i < 10; ++i)
                             {
                                 results.emplace_back(tp.AddTask(TestTask, -i, -i));
                             }
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
                         });
    std::this_thread::sleep_until(now);
    std::vector<std::future<int>> results2;
    for (int i = 0; i < 10; ++i)
    {
        results2.emplace_back(tp.AddTask(TestTask, i, i));
    }
    for (auto &result : results2)
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
    f.join();
    tp.Stop();
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

        for (;;)
        {
            int i;
            std::cin >> i;
            switch (i) {
            case 1:
                case1();
                break;
            case 2:
                case2();
                break;
            case 3:
                case3();
                break;
            case 4:
                case4();
                break;
            case 5:
                case5();
                break;
            case 6:
                case6();
                break;
            case 7:
                case7();
                break;
            case 9:
                case9();
                break;
            case 10:
                case10();
                break;
            case 11:
                case11();
                break;
            default:
                return true;
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
