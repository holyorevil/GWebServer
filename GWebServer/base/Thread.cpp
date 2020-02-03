#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "CurrentThread.h"
#include "Logging.h"
//这个源文件是CurrentThread.h和Thread.h两个头文件共同的源文件，也就是说在前两个头文件中声明的函数都在这个源文件中定义。
//一开始看这三个文件被搞得有点懵，细想了一下确实没问题。头文件只是一个声明，当其他源文件引用头文件以后，会在所有被编译的源文件中去找对应的
//定义，所以没有问题。
//并且CurrentThread中的ThreadPid变量可以被多个类使用，所以选择作为全局变量，而不是作为某一个类的成员变量
namespace CurrentThread
{
    __thread pid_t ThreadPid = 0;
}
pid_t CurrentThread::getpid()
{
    if (ThreadPid == 0) 
        ThreadPid = ::syscall(SYS_gettid);
    return ThreadPid;
}

Thread::Thread(const ThreadCallback &func):pthreadId(0),
                                           pid(0),
                                           Callback(func),
                                           started(false){}
Thread::~Thread(){}

int Thread::start()
{
    started = true;
    int ret = ::pthread_create(&pthreadId, NULL, thr, this);
    if (ret)
    {
        started=false;
        perror("pthread_create error\n");
        return -1;
    }
    return 0;
}

int Thread::join()
{
    int ret = ::pthread_join(pthreadId, NULL);
    if (ret)
    {
        perror("pthread_join error\n");
        return -1;
    }
    return 0;
}

void Thread::runInThread()
{
    pid = CurrentThread::getpid();
    //LOG_INFO<<"pid:"<<pid<<" started!";
    if (Callback) Callback();
}

void *Thread::thr(void *thread)
{
    Thread* tempThread=static_cast<Thread*>(thread);
    //ThreadCallback cb=tempThread->GetCallback();
    //if (cb) cb();/*不创建runInThread就需要这么写，比较麻烦*/
    tempThread->runInThread();
    return NULL;
}