#include "threadpool.h"

#include "logger.h"
#include <future>

ThreadPool::~ThreadPool()
{
    if (isRunning_)
    {
        Stop();
    }
}

bool ThreadPool::Start()
{
    {
        std::shared_lock<std::shared_mutex> lock(statusMutex_);
        if (isRunning_)
        {
            warn("thread pool {} is already running", name_);
            return false;
        }
    }

    {
        std::unique_lock<std::shared_mutex> lock(statusMutex_);
        isRunning_ = true;
    }

    for (std::size_t i = 0; i < threadNum_; ++i)
    {
        auto f = [this, i]
        {
            std::string threadName = this->name_ + "[" + std::to_string(i) + "]";
            info("thread {} is running", threadName);

            for (;;)
            {

                std::shared_lock<std::shared_mutex> statusLock(this->statusMutex_);
                if (!this->isRunning_)
                {
                    break;
                }

                std::packaged_task<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->taskMutex_);
                    while (this->tasks_.empty())
                    {
                        this->taskCond_.wait(lock);
                        if (!this->isRunning_)
                        {
                            info("thread {} is stopped", threadName);
                            return;
                        }
                    }
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                try
                {
                    task();
                }
                catch (const std::exception &e)
                {
                    error("task {} throws exception: {}", threadName, e.what());
                }
            }

            info("thread {} is stopped", threadName);
        };

        threads_.emplace_back(f);
    }

    info("thread pool {} is started", name_);
    return true;
}

bool ThreadPool::Stop(bool wait)
{
    if (!isRunning_)
    {
        warn("thread pool {} is not running", name_);
        return false;
    }

    if (wait)
    {
        {
            std::unique_lock<std::shared_mutex> statusLock(statusMutex_);
            isPreparingToStop_ = true;
        }
        // 只读操作，不需要加锁
        while (!tasks_.empty())
        {
            // 等待任务消耗完毕
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        {
            std::unique_lock<std::shared_mutex> statusLock(statusMutex_);
            isRunning_ = false;
        }
    }
    else
    {
        {
            std::unique_lock<std::shared_mutex> statusLock(statusMutex_);
            isPreparingToStop_ = true;
            isRunning_ = false;
        }
    }

    taskCond_.notify_all();
    for (auto &thread : threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    if (!wait)
    {
        
    }

    info("thread pool {} is stopped", name_);
    return true;
}