
#ifndef _BLOCKINGQUEUE_H_
#define _BLOCKINGQUEUE_H_

#include "Mutex.h"
#include "Condition.h"

#include "deque"

template <typename T>
class BlockingQueue : noncopyable
{
public:
using queue_type = std::deque<T>;
    BlockingQueue():
    mutex_(),
    cond_(mutex_),
    queue_()
    {
    }
    ~BlockingQueue();

    void put(const T& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        cond_.notify();
    }

    void put(const T&& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        cond_.notify();
    }

    T take() {
         MutexLockGuard lock(mutex_);
         while(queue_.empty()) {
            cond_.wait();
         }
         T  data = queue_.front();
         queue_.pop_front();
         return data;
    }

    queue_type drain() {
        queue_type que;
        {
            MutexLockGuard lock(mutex_);
            que = std::move(queue_);
        }
        return que;
    }

    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
private:
    mutable MutexLock mutex_;
    Condition   cond_    GUARDED_BY(mutex_);
    queue_type  queue_ GUARDED_BY(mutex_); // clang 编译器支持
};



#endif