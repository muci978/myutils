#include <memory>
#include <utility>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{
private:
    struct Node
    {
        std::shared_ptr<T> data = nullptr;
        std::unique_ptr<Node> next = nullptr;
    };

public:
    ThreadSafeQueue() : head_(std::make_unique<Node>()), tail_(head_.get()) {}
    template <typename U>
    void WaitAndPush(U &&value);
    template <typename U>
    bool TryPush(U &&value);
    std::shared_ptr<T> WaitAndPop();
    bool TryPop(std::shared_ptr<T> &result);
    bool Empty()
    {
        std::lock_guard<std::mutex> headLock(headMutex_);
        std::lock_guard<std::mutex> tailLock(tailMutex_);
        return head_.get() == tail_;
    }

private:
    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue(ThreadSafeQueue &&other) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue &operator=(ThreadSafeQueue &&other) = delete;

private:
    std::unique_ptr<Node> head_;
    Node *tail_;
    std::mutex headMutex_;
    std::mutex tailMutex_;
    std::condition_variable dataCond_;
};

template <typename T>
template <typename U>
void ThreadSafeQueue<T>::WaitAndPush(U &&value)
{
    std::shared_ptr<T> data(new T(std::forward<U>(value)));
    std::unique_ptr<Node> newTail(new Node);
    {
        std::lock_guard<std::mutex> tailLock(tailMutex_);
        tail_->data = std::move(data);
        tail_->next = std::move(newTail);
        tail_ = tail_->next.get();
    }
    dataCond_.notify_one();
}

template <typename T>
template <typename U>
bool ThreadSafeQueue<T>::TryPush(U &&value)
{
    std::shared_ptr<T> data(new T(std::forward<U>(value)));
    std::unique_ptr<Node> newTail(new Node);
    std::unique_lock<std::mutex> tailLock(tailMutex_, std::try_to_lock);
    if (tailLock.owns_lock())
    {
        tail_->data = std::move(data);
        tail_->next = std::move(newTail);
        tail_ = tail_->next.get();
        tailLock.unlock();
        dataCond_.notify_one();
        return true;
    }
    return false;
}

template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::WaitAndPop()
{
    std::unique_lock<std::mutex> headLock(headMutex_);
    dataCond_.wait(headLock, [this]
                   {
                       std::lock_guard<std::mutex> tailLock(tailMutex_);
                       return tail_ != head_.get();
                   });
    std::shared_ptr<T> result(std::move(head_->data));
    head_ = std::move(head_->next);
    return result;
}

template <typename T>
bool ThreadSafeQueue<T>::TryPop(std::shared_ptr<T> &result)
{
    std::unique_lock<std::mutex> headLock(headMutex_, std::try_to_lock);
    if (headLock.owns_lock())
    {
        std::unique_lock<std::mutex> tailLock(tailMutex_, std::try_to_lock);
        if (!tailLock.owns_lock() || head_.get() == tail_)
        {
            return false;
        }
        tailLock.unlock();
        result = std::move(head_->data);
        head_ = std::move(head_->next);
        return true;
    }
    return false;
}