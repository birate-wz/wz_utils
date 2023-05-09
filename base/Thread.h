

#ifndef __THREAD_H__
#define __THREAD_H__

#include "noncopyable.h"
#include "Atomic.h"
#include "CountDownLatch.h"

#include "functional"
#include "memory"
#include <pthread.h>


class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    void start();
    void run();
    int join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_.get(); }
private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t  pthreadID_;
    pid_t tid_;
    ThreadFunc func_;
    std::string  name_;
    static AtomicInt32 numCreated_;
    CountDownLatch latch_;
};



#endif 