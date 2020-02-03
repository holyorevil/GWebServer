#include "Timer.h"
#include "HttpConnect.h"
#include <sys/time.h>
#include <iostream>
#include "EventLoop.h"
const static int kMicroSecondsPerSecond = 1000 * 1000;

TimerNode::TimerNode(std::shared_ptr<HttpConnect> httpPtr_, int timeout):
        deleted(false),httpPtr(httpPtr_)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    expiredTime = tv.tv_sec*1000+tv.tv_nsec/kMicroSecondsPerSecond+timeout;
}

TimerNode::TimerNode(TimerNode &copy):expiredTime(0)
{
    httpPtr=copy.httpPtr;
}

TimerNode::~TimerNode()//如果绑定在HttpConnect对象上的定时器到时间了，那么也会将HttpConnect析构掉
{
    if(httpPtr) httpPtr->GetLoop()->runInLoop(std::bind(&HttpConnect::handleClose,httpPtr));
}

void TimerNode::update(int timeout)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    expiredTime = tv.tv_sec*1000+tv.tv_nsec/kMicroSecondsPerSecond+timeout;
}

bool TimerNode::isValid()
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    size_t now = tv.tv_sec*1000+tv.tv_nsec/kMicroSecondsPerSecond;
    if (now<expiredTime)
        return true;
    else {
        setDeleted();
        return false;
    }
}

void TimerNode::clearReq()
{
    httpPtr.reset();
    setDeleted();
}


void TimerQueue::addTimer(std::shared_ptr<HttpConnect> SPHttpData, int timeout)
{
    TimerNodePtr newNode(new TimerNode(SPHttpData, timeout));
    newNode->isValid();
    if (newNode->isDeleted()) 
    {
        newNode->clearReq();
        return;
    }
    timerNodeQueue.push(newNode);
    SPHttpData->SetTimerNode(newNode);
}

void TimerQueue::handleExpiredEvent()
{
    while (!timerNodeQueue.empty())
    {
        TimerNodePtr tempNode=timerNodeQueue.top();
        if (tempNode->isDeleted())
        {
            timerNodeQueue.pop();
            //std::cout<<"超时退出\n";
        }
        else
        {
            if (tempNode->isValid()) return;
            else 
            {
                timerNodeQueue.pop();
                //std::cout<<"超时退出\n";
            }
        }
        
    }
}