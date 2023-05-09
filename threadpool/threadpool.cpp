#include "../inc/threadpool.h"

using namespace std;
ThreadPool::ThreadPool(ThreadPoolConfig config):config_(config) {
    total_function_num_.store(0);
    waitting_thread_num_.store(0);

    thread_id_.store(0);
    is_shutdown_.store(false);
    is_shutdown_now_.store(false);

    if (IsValidConfig(config_)) {
        is_available_.store(true);
    } else {
        is_available_.store(false);
    }
}

ThreadPool::~ThreadPool() { ShutDown(false); }

void ThreadPool::ShutDown(bool state) {
    if (is_available_.load()) {
        if(state) {
            is_shutdown_now_.store(true);
        } else {
            is_shutdown_.store(true);
        }
        task_cv_.notify_all();
        is_available_.store(false);
    }
}

 bool ThreadPool::Reset(ThreadPoolConfig config) {
     if (!IsValidConfig(config)) {
         return false;
     }

     if (config_.core_threads != config.core_threads) {
         return false;
     }

     config_ = config;
     return true;
 }

/**
 * thread_num: the number of thread
 * 1. 如果thread_num < core_threads 直接return
 * 2. 如果thread_num > current_thread  增加线程AddThread;
 * 3. 如果thread_num <= current_thread 移除线程
 */

 void ThreadPool::Resize(int thread_num) {
     if (thread_num < config_.core_threads) return;
     int current_thread_num = worker_threads_.size();

     cout<<"current thread number:"<<current_thread_num<<"resize " << thread_num << endl;

    if (thread_num > current_thread_num) {
        while(thread_num-- > current_thread_num){
            AddThread(GetNextThreadId());
        }
    } else {
        int diff = current_thread_num - thread_num;
        cout<< "diff:"<<diff<<endl;
        auto iter = worker_threads_.begin();
        while(iter != worker_threads_.end()) {
            if (diff == 0) break;

            auto thread_ptr = *iter;
            if (thread_ptr->state.load() == ThreadState::kWaiting) {   //移除等待的线程
                thread_ptr->state.store(ThreadState::kStop);
                --diff;
                iter = worker_threads_.erase(iter);   //从线程池中清除
            } else {
                ++iter;
            }
        }
        task_cv_.notify_all();
    }
 }

 bool ThreadPool::Start() {
     if (!IsAvailable()) {
         return false;
     }

     int core_thread_num = config_.core_threads;
     cout<<"Init thread num " << core_thread_num << endl;

     while(core_thread_num --) {
         AddThread(GetNextThreadId());
     }

     cout<< "Init thread pool end "<<endl;
     return true;
 }


/**
 * AddThread: 向线程池中增加线程
 * id: 每个线程池的专属id标识号
 */
void ThreadPool::AddThread(int id) {
    cout<<"AddThread " << id << endl;
    ThreadWrapperPtr thread_ptr = make_shared<ThreadWrapper>();
    thread_ptr->id.store(id);
    auto func = [this, thread_ptr]() {
        while(1) {
            function<void()> task;
            {
                ThreadPoolLock lock(task_mutex_);
                if (thread_ptr->state == ThreadState::kStop) {
                    cout << "Thread id " << thread_ptr->id.load() << " state stop" << endl;
                    break;
                }

                if (this->is_shutdown_ && this->tasks_.empty()) {
                    cout << "Thread id " << thread_ptr->id.load() << " shutdown" << endl;
                    break;
                }

                if (this->is_shutdown_now_) {
                    cout << "Thread id " << thread_ptr->id.load() << " shutdown now" << endl;
                    break;
                }

                cout<<"Thread id " << thread_ptr->id.load() << " running start" <<endl;
                thread_ptr->state.store(ThreadState::kWaiting);
                ++waitting_thread_num_;

                this->task_cv_.wait(lock, [this, thread_ptr] {
                    return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() || thread_ptr->state.load() == ThreadState::kStop);
                }); 
                bool state = (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() || thread_ptr->state.load() == ThreadState::kStop);
                cout << "state " << state << endl;
                --waitting_thread_num_;
                cout << "Thread id " << thread_ptr->id.load() << " running wait end" << endl;

                thread_ptr->state.store(ThreadState::kRunning);
                task = move(this->tasks_.front());
                this->tasks_.pop();
            }
            task();    //执行task
        }
        cout << "Thread id " << thread_ptr->id.load() << " running end" << endl;
    };

    thread_ptr->ptr = make_shared<thread>(move(func)); 
    if(thread_ptr->ptr->joinable()) {
        thread_ptr->ptr->detach();
    }
    this->worker_threads_.emplace_back(move(thread_ptr));
    //cout<< "Add thread "<< thread_ptr->id.load() <<" Thread pool " <<endl;
}


// template <typename F, typename... Args>
// auto ThreadPool::Run(F &&f, Args &&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {

// }
