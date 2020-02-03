#include "Logging.h"
#include <sys/time.h>


__thread time_t t_lastSecond;
__thread char t_time[32];

const char* LogLevelName[Logger::NUM_LOG] = 
{
    "[INFO]  ",
    "[WARN]  ",
    "[ERROR] ",
};

void defaultOutput(const char* msg, int len)
{
    size_t n = ::fwrite(msg, 1, len, stdout);
    (void)n;//为了在编译时防止出现这样的警告：warning: unused variable ‘n’
}

void defaultFlush()
{
    ::fflush(stdout);
}
//静态成员变量一定要在类外初始化
Logger::outputFunc Logger::output = defaultOutput;
Logger::flushFunc Logger::flush = defaultFlush;

Logger::Logger(LogLevel level_, const char* file_, int line_):impl(level_, file_, line_)
{

}

Logger::~Logger()
{
    impl.stream << "\n";
    const FixedBuffer<kSmallBuffer>& buf(impl.stream.getBuffer());
    output(buf.data(), buf.length());
};


Logger::Impl::Impl(LogLevel level_, const char* file_, int line_):stream(),
                                                                  level(level_),
                                                                  file(file_),
                                                                  line(line_)
{
    formatTime();
    stream << LogLevelName[level] << file << " : " << line << "  ";
}

void Logger::Impl::formatTime()//将当前时间整理成规定格式，并输出到流中
{
    struct timeval tv;
    time_t time;
    ::gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    if (t_lastSecond != time)
    {
        t_lastSecond = time;
        struct tm* p_time = ::localtime(&time);   
        ::strftime(t_time, 26, "%Y-%m-%d %H:%M:%S ", p_time);
    }
    stream << t_time;
}