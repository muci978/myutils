#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <memory>
#include <functional>
#include <string>

// TODO:优先级支持
// TODO:优化原子变量内存序
class ThreadPool
{
    using Task = std::packaged_task<void()>;

public:
    explicit ThreadPool(const std::string &name, std::size_t maxTaskNum = 0, std::size_t threadNum = std::thread::hardware_concurrency())
        : name_(name),
          maxTaskNum_(maxTaskNum),
          threadNum_(threadNum > 0 ? threadNum : 1),
          isRunning_(false),
          isPreparingToStop_(false) {}
    ~ThreadPool();

    bool Start();
    bool Stop(bool wait = false);
    template <typename Func, typename... Args>
    auto AddTask(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
    {
        using RetType = decltype(func(args...));

        auto pRealTask = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        auto ret = pRealTask->get_future();
        {
            std::shared_lock<std::shared_mutex> statusLock(statusMutex_);
            if (!isRunning_)
            {
                std::promise<RetType> promise;
                promise.set_exception(std::make_exception_ptr(std::runtime_error(name_ + " is not running")));
                return promise.get_future();
            }

            if (isPreparingToStop_)
            {
                std::promise<RetType> promise;
                promise.set_exception(std::make_exception_ptr(std::runtime_error(name_ + " prepares to be stopped")));
                return promise.get_future();
            }

            {
                std::lock_guard<std::mutex> lock(taskMutex_);
                if (maxTaskNum_ > 0 && tasks_.size() >= maxTaskNum_)
                {
                    std::promise<RetType> promise;
                    promise.set_exception(std::make_exception_ptr(std::runtime_error(name_ + " is full")));
                    return promise.get_future();
                }

                tasks_.emplace([pRealTask]
                               { (*pRealTask)(); });
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

private:
    std::mutex taskMutex_;
    std::condition_variable taskCond_;
    std::queue<Task> tasks_;
    std::vector<std::thread> threads_;
    std::size_t maxTaskNum_; // 0: 无限制，>0: 限制最大任务数
    std::size_t threadNum_;
    std::string name_;
    std::shared_mutex statusMutex_;
    bool isRunning_;
    bool isPreparingToStop_;
};

#endif // THREADPOOL_H