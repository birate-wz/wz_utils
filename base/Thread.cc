#include "Thread.h"

#include "type_traits"
#include "errno.h"
#include "stdio.h"
#include "unistd.h"
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc func, const std::string& name) :
    started_(false),
    joined_(false),
    pthreadID_(0),
    tid_(0),
    func_(std::move(func)),
    name_(name),
    latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadID_);
  }
}
void Thread::run()
{
    tid_ = static_cast<pid_t>(::syscall(SYS_gettid));
    latch_.countDown();
    name_ =  "muduoThread";
    ::prctl(PR_SET_NAME, name_.c_str());
    func_();
}
void Thread::setDefaultName() {
    int num = numCreated_.incrementAndGet();
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Tread%d", num);
        name_ = buf;
    }
}

void* startThread(void *data) {
    Thread *thread = static_cast<Thread*>(data);
    thread->run();
    return nullptr;
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    if (pthread_create(&pthreadID_, NULL, startThread, this)) {
        started_ = false;
    } else {
        latch_.wait();
    }
}

int Thread::join() {
    if (!started_ || joined_)  return false;
    joined_ = true;
    return pthread_join(pthreadID_, NULL);
}
