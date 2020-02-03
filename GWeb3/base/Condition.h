#pragma once
#include "noncopyable.h"
#include "Mutex.h"
#include <stdio.h>
#include "errno.h"
class Condition : public noncopyable
{
public:
    Condition(MutexLock &mutex_):mutex(mutex_){pthread_cond_init(&cond, NULL);}
    ~Condition(){pthread_cond_destroy(&cond);}
    void wait()
    {
        int ret = pthread_cond_wait(&cond, &(mutex.GetMutex()));
        if (ret) perror("pthread_cond_wait error");
    }
    bool waitTime(int seconds)//条件等待seconds，如果在seconds秒内还没有事件发生，就返回true，否则返回false
    {
        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec+=seconds;
        return ETIMEDOUT==pthread_cond_timedwait(&cond, &mutex.GetMutex(), &time);
    }
    void signal()
    {
        int ret = pthread_cond_signal(&cond);
        if (ret) perror("pthread_cond_signal error");
    }
    void broadcast()
    {
        int ret = pthread_cond_broadcast(&cond);
        if (ret) perror("pthread_cond_signal error");
    }
private:
    MutexLock& mutex;//左值引用变量
    pthread_cond_t cond;
};