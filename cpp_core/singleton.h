#ifndef SINGLETON_H
#define SINGLETON_H

#include <utility>

template <typename T>
class Singleton
{
public:
    template <typename... Args>
    static T& GetInstance(Args&&... args)
    {
        static T instance(std::forward<Args>(args)...);
        return instance;
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}

private:
    Singleton(const Singleton& other) = delete;
    Singleton& operator=(const Singleton& other) = delete;
};

#endif // SINGLETON_H