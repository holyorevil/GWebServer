#include "EventLoopThread.h"
#include <functional>
#include "EventLoop.h"

EventLoopThread::EventLoopThread():loop(NULL),
                                   thread(std::bind(&EventLoopThread::ThreadFunc,this)),
                                   mutex(),
                                   condition(mutex){}

EventLoopThread::~EventLoopThread()
{
    if (loop!=NULL)//loop不等NULL，说明还没有执行到EventLoopThread::ThreadFunc的最后一步，所以从loop循环中释放出来
    {
        loop->loopQuit();
        thread.join();
    }
}

EventLoop* EventLoopThread::startLoopThread()
{
    thread.start();
    {
        MutexGuard lock(mutex);
        while(loop==NULL) condition.wait();
    }
    return loop;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop_;
    {
        MutexGuard lock(mutex);
        loop=&loop_;
        condition.signal();
    }
    loop->loop();
    loop=NULL;
}
