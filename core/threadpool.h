#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <atomic>

// TODO:优先级支持
// TODO:优化原子变量内存序
class ThreadPool
{
    using Task = std::function<void()>;
    using Abandon = std::function<void()>;
    using TaskWithAbandon = std::pair<Task, Abandon>;

    enum class Status
    {
        Stopped,
        Running,
        Stopping
    };

public:
    explicit ThreadPool(const std::string &name, std::size_t maxTaskNum = 0, std::size_t threadNum = std::thread::hardware_concurrency())
        : name_(name),
          maxTaskNum_(maxTaskNum),
          threadNum_(threadNum > 0 ? threadNum : 1),
          status_(Status::Stopped) {}
    ~ThreadPool();

    bool Start();
    bool Stop(bool graceful = false);
    template <typename Func, typename... Args>
    auto AddTask(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
    {
        using RetType = decltype(func(args...));

        auto pRealTask = std::make_shared<std::function<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        auto pTaskPromise = std::make_shared<std::promise<RetType>>();
        auto ret = pTaskPromise->get_future();
        {
            if (status_.load(std::memory_order_acquire) != Status::Running)
            {
                return CreateExceptionFuture<RetType>(name_ + " is not running");
            }

            {
                // 在状态变化前进行的AddTask，任务应该被执行，需要考虑两种情况
                // 1.Stopping：此时进行插入操作一定会被执行
                // 2.Stopped：此时进行插入操作不会被执行，任务需要丢弃
                std::lock_guard<std::mutex> lock(taskMutex_);
                if (status_.load(std::memory_order_acquire) == Status::Stopped)
                {
                    return CreateExceptionFuture<RetType>(name_ + " is not running");
                }
                if (maxTaskNum_ > 0 && tasks_.size() >= maxTaskNum_)
                {
                    return CreateExceptionFuture<RetType>(name_ + " task queue is full");
                }

                tasks_.emplace([pRealTask, pTaskPromise, this]
                               { this->DoAndSetPromise(*pRealTask, *pTaskPromise); },
                               [this, pTaskPromise]
                               {
                                   pTaskPromise->set_exception(std::make_exception_ptr(std::runtime_error(this->name_ + " is stopped")));
                               });
            }
        }

        taskCond_.notify_one();
        return ret;
    }

private:
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    template <typename RetType>
    std::future<RetType> CreateExceptionFuture(const std::string &msg)
    {
        std::promise<RetType> promise;
        promise.set_exception(std::make_exception_ptr(std::runtime_error(msg)));
        return promise.get_future();
    }

    // 当函数范围类型为void时，需要特殊处理
    // 此处通过函数匹配规则实现重载
    template <typename Func>
    void DoAndSetPromise(Func &func, std::promise<decltype(func())> &promise)
    {
        promise.set_value(func());
    }

    void DoAndSetPromise(std::function<void()> &func, std::promise<void> &promise)
    {
        func();
        promise.set_value();
    }

private:
    std::mutex taskMutex_;
    std::condition_variable taskCond_;
    std::condition_variable taskEmptyCond_;
    std::queue<TaskWithAbandon> tasks_;
    std::vector<std::thread> threads_;
    std::size_t maxTaskNum_; // 0: 无限制，>0: 限制最大任务数
    std::size_t threadNum_;
    std::string name_;
    std::atomic<Status> status_; // 线程池状态，也可以看做任务队列状态
};

#endif // THREADPOOL_H