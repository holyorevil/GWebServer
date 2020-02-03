#pragma once
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
class HttpListen;

class Server
{
public:
    Server(EventLoop* loop_, int loopNum_, int port_=8080);
    void start();

private:
    EventLoop* loop;//个人理解这里用普通指针而不是智能指针，因为这个Server类是一直随着程序的存在而存在的，
                         //不会在程序结束之前析构的，所以不需要使用智能指针
    int port;
    int socketfd;//连接描述符
    std::shared_ptr<HttpListen> httpListen;
    void newConnection(int socketfd);//这个函数是在HttpListen类中readhandle中运行的，那为什么要放在这里，是因为这个个函数中需要用到EventLoopThreadPool对象
                         //而这个对象属于Server的私有变量，并且并不想把这个变量暴露给其他类，因为一个线程池如果让一个Channel拥有，我觉得不妥
    int loopNum;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool;
};