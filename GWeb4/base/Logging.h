#include "LogStream.h"
#include <memory>
class Logger
{
public:
    enum LogLevel
    {
        INFO,
        WARN,
        ERROR,
        NUM_LOG//这个枚举值是3，代表了前面总共有三种日志等级，为了更好的定义数组
    };
    Logger(LogLevel level_, const char* file_, int line_);
    ~Logger();
    LogStream& stream(){return impl.stream;};

    typedef std::function<void(const char*, int)> outputFunc;
    typedef std::function<void()> flushFunc;
    static void setOutputFunc(const outputFunc& output_){output = output_;}
    static void setFlushFunc(const flushFunc& flush_){flush = flush_;}
private:
    struct Impl
    {
        Impl(LogLevel level_, const char* file_, int line_);
        void formatTime();
        LogStream stream;//日志流
        //每条日志中一定存在的信息
        LogLevel level;//日志等级
        std::string file;//日志产生的文件
        int line;//日志产生的行数
    };
    struct Impl impl;

    static outputFunc output;
    static flushFunc flush;
};

#define LOG_INFO Logger(Logger::INFO, __FILE__, __LINE__).stream()
#define LOG_WARN Logger(Logger::WARN, __FILE__, __LINE__).stream()
#define LOG_ERROR Logger(Logger::ERROR, __FILE__, __LINE__).stream()