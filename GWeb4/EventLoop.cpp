#include "EventLoop.h"
#include <iostream>
#include <sys/eventfd.h>
#include "Channel.h"
static int createEventfd()
{
    int efd = ::eventfd(0, EFD_CLOEXEC|EFD_CLOEXEC);
    if (efd <= 0)
    {
        perror("eventfd error");
        return -1;
    }
    else return efd;
}

EventLoop::EventLoop():poller(new Epoll),
                       threadID(CurrentThread::getpid()),
                       wakeupfd(createEventfd()),
                       wakeupChannelPtr(new Channel(this, wakeupfd)),
                       doPendingFunctor(false),
                       quit(false)
{
    wakeupChannelPtr->SetEvent(EPOLLIN);
    wakeupChannelPtr->setReadHandler(std::bind(&EventLoop::eventfdHandleRead, this));
    addToPoller(wakeupChannelPtr, 0);//EventLoop构造函数一定在EventLoop所属IO线程中，所以不需要使用runInLoop
};

void EventLoop::loop()
{
    std::vector<ChannelPtr> ActiveEvent;
    while (!quit)
    {
        ActiveEvent.clear();
        int ret = poller->poll(ActiveEvent);
        if (ret < 0)
        {
            std::cout<<"poll error\n";
            break;
        }
        /*处理活跃的文件描述符*/
        for (size_t i=0; i<ActiveEvent.size(); i++)
        {
            ActiveEvent[i]->handleEvents();  
        }
        /*处理定时器超时*/
        poller->handleExpired();
        /*处理任务队列中的函数*/
        doPendingFunctor=true;
        std::vector<Functor> pendingFunctor;
        {
            MutexGuard lock(mutex);
            pendingFunctor.swap(functorQueue);
        }
        for (size_t i=0; i<pendingFunctor.size(); i++)
            pendingFunctor[i]();
        doPendingFunctor=false;
    }
}

void EventLoop::runInLoop(const Functor &Func)
{
    if (isInLoopThread()) Func();
    else queueInLoop(Func);
}

void EventLoop::queueInLoop(const Functor &Func)
{
    {
        MutexGuard lock(mutex);
        functorQueue.push_back(Func);
    }
    /* 唤醒的时机：
     * 1.如果调用queueInLoop的函数不在IO线程中，那么会唤醒，这个唤醒条件是为了与runInLoop配套的
     * 2.如果IO线程正在处理任务队列中的函数时会唤醒，这个比较难理解。
     * 首先这种唤醒条件一定是queueInLoop已经在IO线程中了，否则第一条不通过，就直接唤醒了。
     * IO线程一直是在不停的loop循环中，loop循环主要就是分为三块：poll，处理活跃事件和过期定时器，处理任务队列。
     * 
     * (1)如果在IO线程中调用了queueInLoop，那么肯定不会是poll阻塞时调用的，因为一个线程无法去同时做两件事,那么只可能在后面两个部分
     * (2)如果IO线程如果是在执行处理活跃事件和过期定时器部分，那么接下来就会执行处理任务队列部分，
     * 所以不需要唤醒，只要把Func函数放到任务队列中即可，因为即使唤醒了，也要从poll开始接收到eventfd事件，
     * 然后将循环执行一遍再来到处理任务队列部分去执行该函数。
     * (3)如果在处理任务队列部分，那么就需要唤醒了，如果不唤醒，当前循环结束以后，会一直阻塞在poll处，不会立马去任务队列中执行Func函数
    */
    if (!isInLoopThread()|doPendingFunctor)
        wakeup();
}

void EventLoop::wakeup()
{
    uint64_t one=1;
    int ret = ::write(wakeupfd, (char*)&one, 8);//sizeof(uint64_t)就是8
    if (ret != 8)
    perror("wakeup write error");
}

void EventLoop::eventfdHandleRead()
{
    uint64_t one;
    int ret = ::read(wakeupfd, (char*)&one, 8);
    if (ret!=8)
    perror("eventfdHandleRead error");
}

void EventLoop::loopQuit()
{
    quit=true;
    if (!isInLoopThread())
        wakeup();
        
}