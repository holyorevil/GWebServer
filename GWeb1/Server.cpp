#include "Server.h"
#include "Util.h"
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <functional>
//测试用的回调函数,之后可以删除
namespace
{
void ConnectHandle(int fd, EventLoop* epoll_test, ChannelPtr &channel)
{
    // 读数据
    std::string buf;
    //bool zero=false;
    ssize_t len;
    // 从客户端读取数据
    len=sockets::readn(fd, buf);
    if(len == 0)
    {
        printf("客户端断开了连接\n");
        epoll_test->removeFromPoller(channel);
        close(fd);
        return;
    }
    else if(len == -1)
    {
        if(errno == EAGAIN)
        {
            printf("缓冲区数据已经读完\n");
            return;
        }
        else
        {
            printf("recv error----\n");
            close(fd);
            return;
        }
    }
    write(STDOUT_FILENO, buf.c_str(), len);
    sockets::writen(fd, buf);
}

void ConnectError(EventLoop* epoll_test, ChannelPtr &channel)
{
    printf("连接出现错误\n");
    epoll_test->removeFromPoller(channel);
    close(channel->GetFd());
}

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
    std::cout<<"connect success";
    ChannelPtr ConnectChannel(new Channel(connfd));
    ConnectChannel->SetEvent(EPOLLIN|EPOLLET);
    ConnectChannel->setReadHandler(std::bind(&ConnectHandle, connfd, epoll_test, ConnectChannel));
    ConnectChannel->setErrorHandler(std::bind(&ConnectError, epoll_test, ConnectChannel));
    epoll_test->addToPoller(ConnectChannel);
    /*打印客户端信息*/
    char ip[64] = {0};
    printf("New Client IP: %s, Port: %d\n",
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(client_addr.sin_port));
}
}


Server::Server(EventLoop* loop_, int port_):loop(loop_),port(port_),SocketChannel(new Channel)
{
    if ((socketfd = sockets::socket_bind_listen(port))>0)//比较运算符和等于运算符，会先执行比较运算符，所以不加括号的话，socketfd就只有0和1两种值
        std::cout<<"socket_bind_listen success\n";
    else 
    {
        std::cout<<socketfd<<"socket_bind_listen error";
        return;
    }
    SocketChannel->SetFd(socketfd);
    sockets::handle_for_sigpipe();
}

void Server::start()
{
    SocketChannel->SetEvent(EPOLLIN);
    SocketChannel->setReadHandler(std::bind(&SocketHandle, socketfd, loop));
    loop->addToPoller(SocketChannel);
    loop->loop();
    close(socketfd);
}