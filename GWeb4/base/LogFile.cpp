#include "LogFile.h"
#include <iostream>
#include "Mutex.h"
#define kCheckTimeRoll 1024
#define kRollPerSeconds 60*60*24



/*File::File(const std::string &filename):fp(::fopen(filename.c_str(), "ae"))//a表示在打开的文件末尾追加，而e表示O_CLOEXEC
{
    if (fp == NULL)
    {
        std::cout<<"fopen fail\n";
        return;
    }
    //设置fp缓冲区大小
    ::setbuffer(fp, buffer, sizeof(buffer));
}

File::~File()
{
    ::fclose(fp);
}

void File::append(const char* content, size_t length)
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

size_t File::unlockWrite(const char* content, size_t length)
{
    return ::fwrite_unlocked(content, 1, length, fp);
}*/



LogFile::LogFile(const std::string& base, int flush):basename(base), 
                                                     flushInterval(flush), 
                                                     startTime(0),
                                                     lastFlush(0),
                                                     mutex(new MutexLock())
{
    rollFile();
}

void LogFile::append(const char* logline, int len)
{
    MutexGuard lock(*mutex);
    append_unlocked(logline, len);
}

void LogFile::append_unlocked(const char* content, size_t length)
{
    file->append(content, length);//先将数据添加到缓存中
    //下面判断需不需要刷新或者回滚日志
    time_t now = ::time(NULL);
    time_t nowDay = now / kRollPerSeconds * kRollPerSeconds;
    if (nowDay != startTime) rollFile();
    else if(now - lastFlush > flushInterval)
    {
        lastFlush = now;
        file->flush();
    }
}

void LogFile::flush()
{
    MutexGuard lock(*mutex);
    file->flush();
}


//回滚日志文件，这里回滚的意思就是：
//如果当下打开一个日志文件，那么就关闭这个文件，并且重新创建一个文件，
//如果没有打开，就直接创建一个新的日志文件
void LogFile::rollFile()
{
    std::string logName = setLogName();
    file.reset(new File(logName));
    startTime = startTime / kRollPerSeconds * kRollPerSeconds;//将时间调整到当天零点
}

//设置日志文件的名字,日志名称格式是：basename + 年月日-小时分秒 + .log
std::string LogFile::setLogName()
{
    std::string filename(basename);

    char timebuf[32];
    struct tm tm;
    startTime = time(NULL);
    gmtime_r(&startTime, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, "_%Y%m%d-%H%M%S_", &tm);//将事件按照格式化来输出字符串

    filename += timebuf;
    filename += ".log";
    return filename;
}