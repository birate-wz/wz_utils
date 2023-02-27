#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "Mutex.h"
#include "Thread.h"
#include "Condition.h"

#include <deque>
#include <vector>
class ThreadPool
{
public:
    using Task = std::function<void()>;
    explicit ThreadPool(const std::string &name = std::string("ThreadPool"));
    ~ThreadPool();

    void start(int numthreads);
    void stop();

    const std::string& getThreadPoolName() {
        return name_;
    }

    size_t getQueueSize() const {
        return taskQueue_.size();
    }

    void run(Task f);
private:
    void runThread();
    MutexLock mutex_;
    std::vector<std::unique_ptr<Thread>> threads_
    std::deque<Task> taskQueue_;
    bool running_;
    std::string name_;

    Condition notEmpty_ GUARDED_BY(mutex_);
    Condition notFull_ GUARDED_BY(mutex_);
};







#endif