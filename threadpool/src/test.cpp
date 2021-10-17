#include "../inc/threadpool.h"

#include <iostream>

void TestThreadPool() {
    cout<<"hello" <<endl;

    ThreadPool pool(ThreadPool::ThreadPoolConfig{4, 5, 6, chrono::seconds(4)});
    pool.Start();
    this_thread::sleep_for(chrono::seconds(4));
    cout << "thread size " << pool.GetTotalThreadSize() << endl;

    atomic<int> index;
    index.store(0);

    thread t([&]() {
        for(int i = 0; i < 10; ++i) {
            pool.Run([&]() {   //task
                cout<< "function " << index.load() << endl;
                this_thread::sleep_for(chrono::seconds(4));
                index++;
            });
        }
    });

    t.detach();
    cout<<"===========" <<endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));
    pool.Reset(ThreadPool::ThreadPoolConfig{4, 4, 6, std::chrono::seconds(4)});
    std::this_thread::sleep_for(std::chrono::seconds(4));
    cout << "thread size " << pool.GetTotalThreadSize() << endl;
    cout << "waiting size " << pool.GetWaittingThreadSize() << endl;
    cout << "---------------" << endl;
    // pool.ShutDownNow();
    getchar();
    cout << "world" << endl;
}

int main() {
    TestThreadPool();
    return 0;
}