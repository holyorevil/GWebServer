#include "EventLoopThreadPool.h"
#include "iostream"
EventLoopThreadPool::~EventLoopThreadPool(){}

void EventLoopThreadPool::startPool()
{
    for (int i=0; i<num; ++i)
    {
        std::shared_ptr<EventLoopThread> tempthread(new EventLoopThread());
        threadPool.push_back(tempthread);
        loopPool.push_back(tempthread->startLoopThread());
    }
}

EventLoop* EventLoopThreadPool::getLoop()
{
    EventLoop* loop = baseloop;//如果线程池没有内存，就使用主线程
    if (loopPool.size()>turn)
    {
        std::cout<<"使用EventLoop池中第"<<turn<<"个线程"<<std::endl;
        loop=loopPool[turn];
        turn=(turn+1)%num;
    }
    return loop;
}