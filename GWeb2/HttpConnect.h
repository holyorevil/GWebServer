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
    ~HttpConnect(){::close(connfd);};
    std::shared_ptr<Channel> GetChannel(){return ConnectChannel;};
    void SetTimerNode(std::shared_ptr<TimerNode> timerNode_){timerNode=timerNode_;};
    void handleClose();

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