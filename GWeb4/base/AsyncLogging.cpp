#include "AsyncLogging.h"
#include <memory>
AsyncLogging::AsyncLogging(const std::string& logBasename_, int flush):running(false),
                                                                       flushInterval(flush),
                                                                       logBasename(logBasename_),
                                                                       thread(std::bind(&AsyncLogging::threadFunc, this)),
                                                                       latch(1),
                                                                       mutex(),
                                                                       condition(mutex),                                                         
                                                                       currentBuffer(new Buffer),
                                                                       nextBuffer(new Buffer),
                                                                       buffers()
                             
{
    currentBuffer->bzero();
    nextBuffer->bzero();
    buffers.reserve(16);
}
AsyncLogging::~AsyncLogging()
{
    running = false;
    condition.signal();
    thread.join();
}
void AsyncLogging::start()
{
    running = true;
    thread.start();
    latch.wait();//开启后端线程，并主线程阻塞条件等待，等到后端线程进入线程处理函数以后再解除阻塞
}

void AsyncLogging::append(const char* logline, int len)
{
    MutexGuard lock(mutex);
    
    if (currentBuffer->avail() > len) currentBuffer->append(logline, len);
    else//当前缓存块不足的情况下 
    {
        buffers.push_back(std::move(currentBuffer));
        if (nextBuffer) 
            currentBuffer = std::move(nextBuffer);//如果有备用缓存块，就取下拿出来
        else 
            currentBuffer.reset(new Buffer);//如果没有，就创建一个新的
        currentBuffer->append(logline, len);
        condition.signal();
    }
}

void AsyncLogging::threadFunc()
{
    latch.countDown();
    
    LogFile output(logBasename);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    BufferVector buffersToWrite;//这三个都是对应AsyncLogging类中的成员变量，为了将这些成员变量中的内容替换下来，
                                //然后可以一边用这些临时变量往日志文件中写，也可以让其他线程往AsyncLogging类中的成员变量写
    newBuffer1->bzero();
    newBuffer2->bzero();
    buffersToWrite.reserve(16);
    while(running)
    {
        
        {//这段加锁的程序，主要是为了完成等待日志消息写入缓存，并将缓存从成员变量中置换出来的过程
            MutexGuard lock(mutex);
            if (buffers.empty()) 
                condition.waitTime(flushInterval);

            buffers.swap(buffersToWrite);
            buffersToWrite.push_back(std::move(currentBuffer));
            currentBuffer = std::move(newBuffer1);

            if (!nextBuffer)//因为nextBuffer中是不会存数据的，
                            //如果currentBuffer用完了，会将nextBuffer拿过去变为currentBuffer去使用，而不是在nextBuffer中继续写
                            //所以nextBuffer中length一定大于零，nextBuffer只有两种状态，一种是空，说明日志较多，用了两块日志了
                            //还有一种是没有数据的buffer，说明日志较少，没有用到备用Buffer
            {
                nextBuffer = std::move(newBuffer2);
            }
        }
        if (buffersToWrite.size() > 25)
        {
            char tip[] = "log message is too much, droped some of them!\n";
            output.append(tip, static_cast<int>(strlen(tip)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }
        for (size_t i = 0; i < buffersToWrite.size(); ++i)
        {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}