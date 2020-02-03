#include "LogFile.h"
#include "LogStream.h"
#include "Thread.h"
#include "CountDownLatch.h"
#include "Condition.h"
#include "Mutex.h"
#include <vector>
class AsyncLogging : public noncopyable
{
public:
    AsyncLogging(const std::string& logBasename_, int flush = 3);
    ~AsyncLogging();
    void start();
    void append(const char* logline, int len);
private:
    bool running;//用于循环判断的标志位
    int flushInterval;//刷新间隔，每次刷新都将双缓冲中的数据写入File类中的fwrite中的缓冲区
    std::string logBasename;//日志的基本名称

    Thread thread;
    CountDownlatch latch;
    MutexLock mutex;
    Condition condition;

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;

    BufferPtr currentBuffer;
    BufferPtr nextBuffer;
    BufferVector buffers; 

    void threadFunc();//后端负责写入日志文件的线程的执行函数
};
