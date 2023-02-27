
#include "ThreadPool.h"


ThreadPool::ThreadPool(const std::string &name):
    mutex_(),
    running_(false),
    name_(name),
    notEmpty_(mutex_),
{

}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll();
        for (auto& thread : threads_) {
            thread->joinThread();
        }
        threads_.clear();
        taskQueue_.clear();
    }
}

void ThreadPool::start(int numthreads) {
    if (numthreads == 0) {
        printf("zero threads");
        return;
    }
    running_ = true;
    threads_.reserve(numthreads);
    for (auto i : numthreads) {
        char id[32];
        snprintf(id, sizeof id, "id", i+1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runThread, this), name_+id));
        threads_[i].start();
    }
}

void run(Task f) {
    MutexLockGuard lock(mutex_);
    taskQueue_.push(std::move(f));
    notEmpty_.notify();
}

void ThreadPool::runThread() {
    while(running_) {
        Task task = [&]() {
            MutexLockGuard lock(mutex_);
            while(taskQueue_.empty() && running_) {
                notEmpty_.wait();
            }
            Task task = taskQueue_.front();
            taskQueue_.pop();
            return task;
        };
        if (task) {
            task();
        }
    }
}