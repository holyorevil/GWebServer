#pragma once
#include "noncopyable.h"
#include <string.h>
#include <string>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;
const int kMaxNumericSize = 32;//日志流中所允许一个数据的最大位数，也是缓存区中需要为一个数据所预留的空间，如果一个缓冲区中
//空闲区间小于32，那么就不会让一个数据存入缓冲区

template<int SIZE>
class FixedBuffer : public noncopyable
{
public:
    FixedBuffer() : cur(buf) {}

    ~FixedBuffer() {}

    void append(const char* buf, size_t len) {//缓冲区中追加内容
        if (avail() > static_cast<int>(len)) {
        memcpy(cur, buf, len);
        cur += len;
        }
    }

    const char* data() const { return buf; }//得到整个缓冲区
    int length() const { return static_cast<int>(cur - buf); }//得到缓冲区中存储数据的长度

    char* current() { return cur; }//得到缓冲区中当前空闲区域的首地址
    int avail() const { return static_cast<int>(end() - cur); }//缓冲区中空闲区域容量
    void add(size_t len) { cur += len; }//将缓冲区中len字节变为已存储空间

    void reset() { cur = buf; }//清空缓冲区
    void bzero() { memset(buf, 0, sizeof buf); }//清零缓冲区
private:
    const char* end() const { return buf + sizeof buf; }

    char buf[SIZE];
    char* cur;//指向缓冲区空闲空间的首地址
};
//LogStream类就是重载了<<符号，让LogStream<<可以使用，无论<<后面跟数字还是字符串，都会被转换成字符的形式，存到LogStream下面的Buffer中
class LogStream : public noncopyable
{
public:
    LogStream& operator<<(bool v);
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(float v);
    LogStream& operator<<(double);
    LogStream& operator<<(long double);
    LogStream& operator<<(char v);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string& v);

    void append(const char* data, int len) { buffer.append(data, len); }
    const FixedBuffer<kSmallBuffer>& getBuffer() const { return buffer; }
    void resetBuffer() { buffer.reset(); }
private:
    FixedBuffer<kSmallBuffer> buffer;

    template <typename T>
    void formatInteger(T);
};