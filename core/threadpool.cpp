#include "threadpool.h"

#include "logger.h"
#include <atomic>

ThreadPool::~ThreadPool()
{
    if (status_.load(std::memory_order_acquire) == Status::Running)
    {
        Stop(false);
    }
}

bool ThreadPool::Start()
{
    // 此时没有worker线程，不需要锁，但是需要防御多个线程同时调用Start
    Status expected = Status::Stopped;
    if (!status_.compare_exchange_strong(expected, Status::Running, std::memory_order_acq_rel, std::memory_order_relaxed))
    {
        warn("thread pool {} is already running", name_);
        return false;
    }

    for (std::size_t i = 0; i < threadNum_; ++i)
    {
        auto f = [this, i]
        {
            std::string threadName = this->name_ + "[" + std::to_string(i) + "]";
            info("thread {} is running", threadName);

            for (;;)
            {
                TaskWithAbandon task;
                {
                    std::unique_lock<std::mutex> lock(this->taskMutex_);
                    this->taskCond_.wait(lock, [this]()
                                         {
                                             Status currentStatus = this->status_.load(std::memory_order_acquire);
                                             return (currentStatus == Status::Running && !this->tasks_.empty()) || (currentStatus != Status::Running); });

                    // 唤醒后直接判断是否停止
                    if (this->status_.load(std::memory_order_acquire) == Status::Stopped)
                    {
                        break;
                    }
                    if (this->status_.load(std::memory_order_relaxed) == Status::Stopping && this->tasks_.empty())
                    {
                        this->taskEmptyCond_.notify_all();
                        this->status_.store(Status::Stopped, std::memory_order_release);
                        break;
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
                    error("{} throws exception: {}", threadName, e.what());
                }
                catch (...)
                {
                    error("{} throws unknown exception", threadName);
                }
            }

            info("thread {} is stopped", threadName);
        };

        threads_.emplace_back(f);
    }

    info("thread pool {} is started", name_);
    return true;
}

bool ThreadPool::Stop(bool graceful)
{
    // 通过原子操作改变状态，防止多个线程同时调用Stop
    Status expected = Status::Running;
    if (!status_.compare_exchange_strong(expected, graceful ? Status::Stopping : Status::Stopped, std::memory_order_acq_rel, std::memory_order_relaxed))
    {
        warn("thread pool {} is not running", name_);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(taskMutex_);
        taskCond_.notify_all();
        taskEmptyCond_.wait(lock, [this]()
                            { return this->status_.load(std::memory_order_acquire) == Status::Stopped || this->tasks_.empty(); });
    }

    for (auto &thread : threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    if (!graceful)
    {
        info("thread pool {} abandon all tasks, {} tasks left", name_, tasks_.size());
        while (!tasks_.empty())
        {
            tasks_.front().second();
            tasks_.pop();
        }
    }

    info("thread pool {} is stopped", name_);
    return true;
}