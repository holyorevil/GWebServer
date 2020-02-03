#pragma once
#include "Epoll.h"
#include "base/CurrentThread.h"
#include "base/Mutex.h"
/* 在单线程中其实就是对Epoll类的进一步封装，能够让epoll使用起来更加简单。
 * 在多线程中，拥有EventLoop的线程被称为IO线程(以下都称为IO线程)。muduo的模式one loop per thread就是说每个线程最多有一个EventLoop类，也有可能没有
 * 这个类实现的功能主要是epoll队列的管理以及任务队列中任务的执行 
 */
class HttpConnect;
class Channel;

class EventLoop
{
public:
    typedef std::function<void ()> Functor;
    EventLoop();
    void loop();
    //对于runInLoop和queueInLoop两个函数，我的理解是，这两个函数都是把Func函数放到eventloop所属的线程下来执行
    //不一定在IO线程的调用runInLoop
    //一定在IO线程的调用queueInLoop，或者所在函数已经被runInLoop调用到了IO线程上的情况下
    //想要IO线程执行除了loop以外的其他函数，都需要先进入任务队列中
    //暂时调用这两个函数的地方都有对epoll队列的操作，或者说对epoll队列上文件描述符操作的地方都最好调用这个，确保是用IO线程进行操作的
    void runInLoop(const Functor &Func);
    void queueInLoop(const Functor &Func);
    bool isInLoopThread(){return threadID == CurrentThread::getpid();}//判断是否在EventLoopThread中
    void loopQuit();

    void wakeup();
    void eventfdHandleRead();


    //对Epoll类中epoll队列的操作，虽然这里多了一层封装，但是由于是在类内定义的，所以是内联函数，其实在EventLoop中的函数封装并没有什么用处
    //主要是如果只使用一层封装，我只想到两种改动，一种是将poller改成共有变量，显然不好，暴露了成员变量，
    //还有一种是这些函数不在Epoll类中定义，而是在EventLoop中定义，这也不好，类的分工就不明确了。所以为了结构的清晰，就选择了牺牲性能的方法
    void removeFromPoller(std::shared_ptr<Channel> channel) {
        poller->epoll_del(channel);
    }
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller->epoll_mod(channel, timeout);
    }
    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller->epoll_add(channel, timeout);
    }
    void updateTimerNode(std::shared_ptr<HttpConnect> channel, int timeout = 0){
        poller->UpdateTimerNode(channel, timeout);
    }
private:
    std::unique_ptr<Epoll> poller;
    pid_t threadID;//EventLoop所属线程ID
    int wakeupfd;//eventfd
    std::shared_ptr<Channel> wakeupChannelPtr;//存放eventfd的Channel
    std::vector<Functor> functorQueue;//需要由IO线程执行的任务队列
    MutexLock mutex;
    bool doPendingFunctor;//是否正在处理任务队列的标志位
    bool quit;//离开标志符
};