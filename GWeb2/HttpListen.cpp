#include "HttpListen.h"
#include "Util.h"
#include "HttpConnect.h"
#include "EventLoop.h"
#include <iostream>
#include <functional>
static const int DEFAULT_EXPIRED_TIME = 2000;              // ms

HttpListen::HttpListen(EventLoop* loop_, int port_):loop(loop_),port(port_),ConnectChannel(new Channel(loop_))
{
    if ((socketfd = sockets::socket_bind(port))>0)//比较运算符和等于运算符，会先执行比较运算符，所以不加括号的话，socketfd就只有0和1两种值
        std::cout<<"socket_bind_listen success\n";
    else 
    {
        std::cout<<socketfd<<"socket_bind_listen error";
        return;
    }
    sockets::handle_for_sigpipe();
    
    ConnectChannel->SetFd(socketfd);
    ConnectChannel->SetEvent(EPOLLIN);
    ConnectChannel->setReadHandler(std::bind(&HttpListen::handleRead, this));
    
}

void HttpListen::Listen()
{
    ConnectChannel->SetUpClass(shared_from_this());
    loop->addToPoller(ConnectChannel, 0);
    sockets::Listen(socketfd);
}


void HttpListen::handleRead()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connfd = ::accept4(socketfd, (sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK|SOCK_CLOEXEC);

    if(connfd == -1)
    {
        perror("accept error");
        abort();
    }
    std::cout<<"connect success\n";
    std::shared_ptr<HttpConnect> tempHttpConnectPtr(new HttpConnect(loop, connfd));
    tempHttpConnectPtr->GetChannel()->SetUpHttpConnect(tempHttpConnectPtr);
    tempHttpConnectPtr->GetChannel()->SetUpClass(tempHttpConnectPtr);
    loop->addToPoller(tempHttpConnectPtr->GetChannel(), DEFAULT_EXPIRED_TIME);
    /*打印客户端信息*/
    char ip[64] = {0};
    printf("New Client IP: %s, Port: %d\n",
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(client_addr.sin_port));
}