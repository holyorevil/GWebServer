#pragma once
#include "base/Thread.h"
#include "base/Mutex.h"
#include "base/Condition.h"

class EventLoop;

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoopThread();
private:
    void ThreadFunc();

    EventLoop* loop;
    Thread thread;
    //CountDownlatch countLock;原来使用的是CountDownlatch来代替mutex和condition，但是这个类只能使用一次，计数器到0就没有用了
    //至于为什么不可以重新设置计数器的值，暂时还不清楚，所以如果使用CountDownlatch类就不可以重复使用startLoopThread，但是使用mutex和condition是可以重复使用的
    MutexLock mutex;
    Condition condition;
};