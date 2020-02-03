#pragma once
#include "Channel.h"
#include <unistd.h>
#include <memory>

class TimerNode;
class EventLoop;

class HttpConnect: public std::enable_shared_from_this<HttpConnect>
{
public:
    HttpConnect(EventLoop* loop_, int connfd_);
    ~HttpConnect(){::close(connfd);}
    std::shared_ptr<Channel> GetChannel(){return ConnectChannel;}
    EventLoop* GetLoop(){return loop;}
    void SetTimerNode(std::shared_ptr<TimerNode> timerNode_){timerNode=timerNode_;}
    void handleClose();
    void connectInit();//这个函数是在与客户端建立连接时调用的，主要是将ConnectChannel挂到对应eventloop上
                       //至于为什么需要另外搞一个函数，是因为创建HttpConnect的线程和管理HttpConnect的epoll队列线程不一定是一个
                       //创建HttpConnect的线程一定是主线程，Server类所在线程，而管理HttpConnect的线程是eventLoopThreadPool中任一个
    void seperateTimer();//将绑定的定时节点timerNode和HttpConnect对象分开
private:
    EventLoop* loop;
    int connfd;
    std::shared_ptr<Channel> ConnectChannel;
    std::weak_ptr<TimerNode> timerNode;//与 HttpConnect对象绑定的计时节点

    void handleRead();
    void handleError();
};

typedef std::shared_ptr<HttpConnect> HttpConnectPtr;