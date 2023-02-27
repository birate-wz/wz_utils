#ifndef __NONCOPYABLE__
#define __NONCOPYABLE__

#include <iostream>

class noncopyable {
   public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

   protected: // 禁止多态调用时delete 基类指针
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif
