#include "HttpListen.h"
#include "Util.h"
#include "HttpConnect.h"
#include "EventLoop.h"
#include <iostream>
#include <functional>

HttpListen::HttpListen(EventLoop* loop_, int port_):loop(loop_),port(port_),ConnectChannel(new Channel(loop_))
{
    if ((socketfd = sockets::socket_bind(port))>0)//比较运算符和等于运算符，会先执行比较运算符，所以不加括号的话，socketfd就只有0和1两种值
        std::cout<<"socket_bind_listen success\n";
    else 
    {
        std::cout<<socketfd<<"socket_bind_listen error\n";
        return;
    }
    sockets::handle_for_sigpipe();
    
    ConnectChannel->SetFd(socketfd);
    ConnectChannel->SetEvent(EPOLLIN);
    ConnectChannel->setReadHandler(std::bind(&HttpListen::handleRead, this));
    
}

void HttpListen::Listen()
{
    loop->addToPoller(ConnectChannel, 0);//HttpListen一定在主线程，也就是Server类所属线程运行，所以不需要runInLoop
    sockets::Listen(socketfd);
}


void HttpListen::handleRead()
{
    if (newConnection)  newConnection(socketfd);
    else close(socketfd);
}