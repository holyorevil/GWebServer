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
/*
namespace{
void SocketHandle(int socketfd, EventLoop* epoll_test)
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
    std::shared_ptr<HttpConnect> tempHttpConnectPtr(new HttpConnect(epoll_test, connfd));
    tempHttpConnectPtr->GetChannel()->SetUpClass(tempHttpConnectPtr);
    epoll_test->addToPoller(tempHttpConnectPtr->GetChannel(), DEFAULT_EXPIRED_TIME);
    //打印客户端信息
    char ip[64] = {0};
    printf("New Client IP: %s, Port: %d\n",
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(client_addr.sin_port));
}
}
*/

Server::Server(EventLoop* loop_, int port_):loop(loop_),port(port_),httpListen(new HttpListen(loop_, port_))
{

}

void Server::start()
{
    httpListen->Listen();
    loop->loop();
}