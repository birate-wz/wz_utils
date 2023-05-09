#ifndef __LOGSTREAM__H
#define __LOGSTREAM__H

#include "noncopyable.h"

#include <string.h> // memcpy

const int kSmallBuffer  = 4096;
const int kLargerBuffer = 4096* 1000;

template <int SIZE>
class  FixedBuffer : noncopyable
{
public:
     FixedBuffer() :
        cur_(data_)
    {
        setCookie(cookieStart);
    }
    ~ FixedBuffer()
    {
        setCookie(cookieEnd);
    }

    void append(const char* buf, size_t len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char *data () const  {  return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    char *current() { return cur_; }
    int avail() const  { return static_cast<int>(end() - cur_ ); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof(data_)); }

    // for used by GDB
    const char* debugString();
    void setCookie(void (*cookie)()) { cookie_ = cookie; }
    // for used by unit test
    std::string toString() const { return std::string(data_, length()); }
    //StringPiece toStringPiece() const { return StringPiece(data_, length()); }
private:
  // Must be outline function for cookies.
  const char* end() const { return data_ + sizeof(data_); }
  static void cookieStart();
  static void cookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};


class LogStream : noncopyable
{
public:
typedef FixedBuffer<kSmallBuffer> Buffer;
    LogStream& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
    return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);
    LogStream& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);

    LogStream& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

    LogStream& operator<<(const char* str)
    {
        if (str)
        {
        buffer_.append(str, strlen(str));
        }
        else
        {
        buffer_.append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    LogStream& operator<<(const Buffer& v)
    {
        *this << v.toString();
        return *this;
    }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 48;
};




#endif
