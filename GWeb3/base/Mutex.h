#pragma once
#include "noncopyable.h"
#include <pthread.h>
class MutexLock : public noncopyable
{
public:
    MutexLock(){pthread_mutex_init(&mutex,NULL);}
    ~MutexLock(){pthread_mutex_destroy(&mutex);}
    void lock(){pthread_mutex_lock(&mutex);}
    void unlock(){pthread_mutex_unlock(&mutex);}
    pthread_mutex_t& GetMutex(){return mutex;}
private:
    pthread_mutex_t mutex;
};

class MutexGuard : public noncopyable
{
public:
    MutexGuard(MutexLock& mutex_):mutex(mutex_)
    {
        mutex.lock();
    }
    ~MutexGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock& mutex;//左值引用传递,在这个类析构时，mutex不会被析构
};