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

                TaskWithExit task;
                {
                    std::unique_lock<std::mutex> lock(this->taskMutex_);
                    // 这里持有statusMutex_的读锁，有无影响？
                    taskCond_.wait(lock, [this]()
                                   {
                                       // 唤醒条件：有任务 或 线程池停止
                                       return !this->tasks_.empty() || !this->isRunning_;
                                   });

                    // 唤醒后直接判断是否停止
                    if (!this->isRunning_)
                    {
                        info("thread {} is stopped", threadName);
                        return;
                    }
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                try
                {
                    task.first();
                }
                catch (const std::exception &e)
                {
                    error("task {} throws exception: {}", threadName, e.what());
                }
                catch (...)
                {
                    error("task {} throws unknown exception", threadName);
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
        // TODO：优化成条件变量
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
        while (!tasks_.empty())
        {
            tasks_.front().second();
            tasks_.pop();
        }
    }

    info("thread pool {} is stopped", name_);
    return true;
}