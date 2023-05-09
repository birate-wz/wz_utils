#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <future>
#include <tuple>
#include <memory>

using namespace std;

class ThreadPool {
public:
    using PoolSecond = chrono::seconds;
    /** 线程池的配置
     * core_threads: 核心线程个数，线程池中最少拥有的线程个数，初始化就会创建好的线程，常驻于线程池
     *
     * max_threads: >=core_threads，当任务的个数太多线程池执行不过来时，
     * 内部就会创建更多的线程用于执行更多的任务，内部线程数不会超过max_threads
     *
     * max_task_size: 内部允许存储的最大任务个数，暂时没有使用
     *
     * time_out: Cache线程的超时时间，Cache线程指的是max_threads-core_threads的线程,
     * 当time_out时间内没有执行任务，此线程就会被自动回收
     */
    struct ThreadPoolConfig {
        int core_threads;
        int max_threads;
        int max_task_size;
        PoolSecond time_out;
    };

    /**
     * 线程的状态: 等待 运行 停止 
     */
    enum class ThreadState {kInit, kWaiting, kRunning, kStop};

    using ThreadPtr = shared_ptr<thread>;
    using ThreadId = atomic<int>;
    using ThreadStateAtomic = atomic<ThreadState>;

    /**
     * 线程池的最小单位，每个线程都有的ID，线程状态
     */

    struct  ThreadWrapper {
        ThreadPtr   ptr;
        ThreadId    id;
        ThreadStateAtomic   state;
        ThreadWrapper () {
            ptr = nullptr;
            id = 0;
            state.store(ThreadState::kInit);
        }
    };

    using ThreadWrapperPtr = shared_ptr<ThreadWrapper>;
    using ThreadPoolLock = unique_lock<mutex>;


public:
    ThreadPool(ThreadPoolConfig config);
    ~ThreadPool();

    bool Reset(ThreadPoolConfig config);

    //获取正在处于等待状态的线程个数
    int GetWaittingThreadSize() { return this->waitting_thread_num_.load(); }
    //获取线程池中的当前线程总个数
    int GetTotalThreadSize() { return this->worker_threads_.size(); }
    //获取当前线程池已经执行过的函数个数
    int GetRunnedFuncNum() { return this->total_function_num_.load(); }
    //当前线程池是否可用
    bool IsAvailable() { return is_available_.load(); }

    bool Start();   //开启线程池

    template <typename F, typename... Args>
    auto Run(F &&f, Args &&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
            if (this->is_shutdown_.load() || this->is_shutdown_now_.load() || !IsAvailable()) {
                return nullptr;
            }

            if (GetWaittingThreadSize() == 0 && GetTotalThreadSize() < config_.max_threads) {
                AddThread(GetNextThreadId());
            }

            using return_type = result_of_t<F(Args...)>;
            auto task = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...));

            total_function_num_++;

            future<return_type> res = task->get_future();
            {
                ThreadPoolLock lock(task_mutex_);
                tasks_.emplace([task]() { (*task)(); });
            }

            this->task_cv_.notify_one();
            return make_shared<future<result_of_t<F(Args...)>>>(move(res));
    } 
    
    //关掉线程池
    //state: 
    //    true  内部还没有执行的任务直接取消，不会再执行
    //    false 内部还没有执行的任务会继续执行
    void ShutDown(bool state);

    //向线程池中增加线程
    void AddThread(int id);
    /**
     * 1. 如果thread_num < core_threads 直接return
     * 2. 如果thread_num > current_thread  增加线程AddThread;
     * 3. 如果thread_num <= current_thread 移除线程
     * 
     */
    void Resize(int thread_num);

private:
    int GetNextThreadId() { return this->thread_id_++; }

    //检查config是否有效
    bool IsValidConfig(ThreadPoolConfig config) {
        return !((config.core_threads < 1 || config.max_threads < config.core_threads || config.time_out.count() < 1));
    }
private:

    ThreadPoolConfig config_;
    list<ThreadWrapperPtr> worker_threads_;  //线程队列

    queue<function<void()>> tasks_;
    mutex  task_mutex_;
    condition_variable task_cv_;

    atomic<int> total_function_num_;
    atomic<int> waitting_thread_num_;
    atomic<int> thread_id_;

    atomic<bool> is_shutdown_now_;
    atomic<bool> is_shutdown_;
    atomic<bool> is_available_;
};




#endif
