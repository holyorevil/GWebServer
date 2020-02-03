#pragma once
#include "EventLoopThread.h"
#include <vector>
#include <memory>
#include "base/noncopyable.h"
class EventLoopThreadPool : public noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseloop_, int num_=0):baseloop(baseloop_),num(num_),turn(0){};
    ~EventLoopThreadPool();
    void startPool();
    EventLoop* getLoop();
private:
    EventLoop* baseloop;
    int num;
    std::vector<std::shared_ptr<EventLoopThread>> threadPool;
    std::vector<EventLoop*> loopPool;
    size_t turn;
};