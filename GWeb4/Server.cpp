#include "Server.h"
#include "Util.h"
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <functional>
#include "HttpConnect.h"
#include "HttpListen.h"
static const int DEFAULT_EXPIRED_TIME = 2000;              // ms

Server::Server(EventLoop* loop_, int loopNum_, int port_):loop(loop_),
                                                          port(port_),
                                                          socketfd(0),
                                                          httpListen(new HttpListen(loop_, port_)),
                                                          loopNum(loopNum_),
                                                          eventLoopThreadPool(new EventLoopThreadPool(loop_, loopNum))                                                                                                                                                                    
{
    httpListen->setNewConnection(std::bind(&Server::newConnection,this,std::placeholders::_1));
}

void Server::start()
{
    httpListen->Listen();
    eventLoopThreadPool->startPool();
    
}

void Server::newConnection(int socketfd)
{
    //这个函数是在主线程中执行的，也就是Server所在的线程
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connfd = ::accept4(socketfd, (sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK|SOCK_CLOEXEC);

    if(connfd == -1)
    {
        perror("accept error");
        abort();
    }
    std::cout<<"connect success\n";
    EventLoop* ioLoop=eventLoopThreadPool->getLoop();
    std::shared_ptr<HttpConnect> tempHttpConnectPtr(new HttpConnect(ioLoop, connfd));
    ioLoop->runInLoop(std::bind(&HttpConnect::connectInit,tempHttpConnectPtr));//这个函数主要用来将tempHttpConnectPtr挂到
    //ioLoop线程上，需要在ioLoop线程下执行，因为我并没有在epoll类中加锁，所以如果对epoll类跨线程操作，可能会出现问题
    //为什么不对epoll类加锁，因为设想这样一种情况，当eventloop线程正在拿着锁poll阻塞时，另一个线程想向这个epoll队列加文件描述符
    //那么这样需要等待很久，等有其他文件描述符发生事件或者阻塞时间到，才会释放锁给其他线程往epoll添加文件描述符，所以epoll队列的添加，管理，阻塞等应该在一个线程中执行
    //而采用runInLoop的方式会唤醒线程，就避免出现上面的情况了。

    /*打印客户端信息*/
    char ip[64] = {0};
    printf("New Client IP: %s, Port: %d\n",
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(client_addr.sin_port));
}