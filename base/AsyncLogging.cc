
#include "AsyncLogging.h"

#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>

// 获取系统当前时间
static std::string getCurrentSystemTime()
{
	auto tt = std::chrono::system_clock::to_time_t
		(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	char date[60] = { 0 };
	sprintf(date, "%d-%02d-%02d-%02d.%02d.%02d",
		(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
		(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
	return std::string(date);
}

AsyncLogging::AsyncLogging(const string& basename,
                            off_t rollSize,
                            int flushInterval):
    flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len) {   // 如果currentbuffer 有空间
        currentBuffer_->append(logline, len);
    } else {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);  // 把current 和 next 交换
        } else {
            currentBuffer_.reset(new Buffer);     // 很少发生
        }
        currentBuffer_.append(logline, len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc()
{
    if (running_ == false) return;
    latch_.countDown();
    std::ofstream output(basename_, ios::out | ios::app);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while(running_) {
        if (newBuffer1 == nullptr || newBuffer1->length() == 0) return;
        if (newBuffer2 == nullptr || newBuffer2->length() == 0) return;
        if (buffersToWrite.empty()) return;

        {
            MutexLockGuard lock(mutex_);
            if (buffers_,empty()) {
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }

        }

        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n", getCurrentSystemTime().c_str(), buffersToWrite.size()-2);
            fputs(buf, stderr);
            output<<buf<<endl;
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite) {
        // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output<<buffer->data()<<endl;
        }
        if (buffersToWrite.size() > 2) {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
    }
}
