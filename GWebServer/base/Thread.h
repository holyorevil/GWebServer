#pragma once
#include <pthread.h>
#include <functional>
#include "noncopyable.h"


class Thread : public noncopyable
{
public: 
    typedef std::function<void()> ThreadCallback;
    explicit Thread(const ThreadCallback &func);
    ~Thread();
    int start();//开启线程
    int join();//阻塞关闭线程
    void runInThread();//这个只是为了在回调函数外面封装一层，
                       //这样在调用回调函数时，不需将Thread::Callback拿出来放到一个临时ThreadCallback对象中去，具体可以参考thr中

    //操作成员变量的函数
    pthread_t GetTid(){return pthreadId;}
    bool GetStarted(){return started;}

private:
    pthread_t pthreadId;//线程ID
    pid_t pid;//线程对应的pid，也就是我博客中所说的第三个ID
    ThreadCallback Callback;//会在线程回调函数中调用，是线程中具体的执行函数，thr只是对这个函数的一个封装
    bool started;

    static void *thr(void *arg);//线程回调函数
};