#pragma once
#include "Condition.h"

class CountDownlatch
{
public:
    CountDownlatch(int num):mutex(),cond(mutex),count(num){};
    ~CountDownlatch(){};
    void wait()
    {
        MutexGuard lock(mutex);
        while(count>0) cond.wait();
    }
    void countDown()
    {
        MutexGuard lock(mutex);
        count--;
        if (count==0) cond.broadcast();
    }
private:
    MutexLock mutex;
    Condition cond;
    int count;
};