#pragma once
#include <stdio.h>
#include <string>
#include <memory>
#include <iostream>
class MutexLock;
/*这两个类主要是实现了后端(IO操作线程)往磁盘上写的过程*/
/*File就是最直接的对磁盘上的日志文件进行操作的类*/

//之所以把File类的定义直接写在头文件处，是因为File类中的函数会被经常调用，所以用内联函数效率更高，所以需要在类内定义
class File
{
public:
    File(const std::string &filename):fp(::fopen(filename.c_str(), "ae"))
    {
        if (fp == NULL)
        {
            std::cout<<"fopen fail\n";
            return;
        }
        //设置fp缓冲区大小
        ::setbuffer(fp, buffer, sizeof(buffer));
    }
    ~File()
    {
        ::fclose(fp);
    }
    void append(const char* content, size_t length)
    {
        size_t n = unlockWrite(content, length);
        size_t remain = length - n;
        while (remain > 0)
        {
            size_t x = unlockWrite(content + n, remain);//这里写之所以使用无锁，是因为在调用这个append函数时，会自己加锁，所以不需要底层再去加一道锁，浪费资源
            if (x == 0) 
            {
                int err = ferror(fp);
                if (err) fprintf(stderr, "AppendFile::append() failed !\n");
                break;
            }
            n += x;
            remain = length - n;
        }
    }
    void flush(){::fflush(fp);}


private:
    FILE* fp;
    char buffer[64*1024];

    size_t unlockWrite(const char* content, size_t length)
    {
        return ::fwrite_unlocked(content, 1, length, fp);
    }
};

/* LogFile是对File的进一步封装，主要实现了一下几个功能：
*  创建统一格式的日志
*  如果隔一段时间，会自动刷新用户层缓冲区
*  如果日志创建时间和当前时间不在一天时，会回滚日志
*/
class LogFile
{
public:
    LogFile(const std::string& base, int flush = 3);
    ~LogFile(){}
    void rollFile();
    void append(const char* logline, int len);
    void flush();


private:
    const std::string basename;//日志的基本名字
    const int flushInterval;//日志刷新时间
    time_t startTime;//日志开始创建的时间
    time_t lastFlush;//上一次刷新时间

    std::unique_ptr<File> file;//File类
    std::unique_ptr<MutexLock> mutex;

    std::string setLogName();
    void append_unlocked(const char* content, size_t length);



};