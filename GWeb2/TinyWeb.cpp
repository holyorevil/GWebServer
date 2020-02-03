#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
 
int main(int argc, const char* argv[])
{
    if(argc < 2)
    {
        printf("eg: ./a.out port\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    int port = atoi(argv[1]);
 
    // 创建套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化服务器 sockaddr_in 
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;                   // 地址族 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 监听本机所有的IP
    serv_addr.sin_port = htons(port);            // 设置端口 
    // 绑定IP和端口
    bind(lfd, (struct sockaddr*)&serv_addr, serv_len);
 
    // 设置同时监听的最大个数
    listen(lfd, 36);
    printf("Start accept ......\n");
 
    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);
 
    // 创建epoll树根节点
    int epfd = epoll_create(2000);
    // 初始化epoll树
    struct epoll_event ev;
 
    // 设置边沿触发
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
 
    struct epoll_event all[2000];
    while(1)
    {
        // 使用epoll通知内核fd 文件IO检测
        int ret = epoll_wait(epfd, all, sizeof(all)/sizeof(all[0]), -1);
        printf("================== epoll_wait =============\n");
 
        // 遍历all数组中的前ret个元素
        for(int i=0; i<ret; ++i)
        {
            int fd = all[i].data.fd;
            // 判断是否有新连接
            if(fd == lfd)
            {
                // 接受连接请求
                int cfd = accept(lfd, (struct sockaddr*)&client_addr, &cli_len);
                if(cfd == -1)
                {
                    perror("accept error");
                    exit(1);
                }
                // 设置文件cfd为非阻塞模式
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);
 
                // 将新得到的cfd挂到树上
                struct epoll_event temp;
                // 设置边沿触发
                temp.events = EPOLLIN | EPOLLET;
                temp.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &temp);
                
                // 打印客户端信息
                char ip[64] = {0};
                printf("New Client IP: %s, Port: %d\n",
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
                    ntohs(client_addr.sin_port));
                
            }
            else
            {
                // 处理已经连接的客户端发送过来的数据
                if(!all[i].events & EPOLLIN) 
                {
                    continue;
                }
 
                // 读数据
                char buf[5] = {0};
                int len;
                // 循环读数据
                while( (len = recv(fd, buf, sizeof(buf), 0)) > 0 )
                {
                    // 数据打印到终端
                    write(STDOUT_FILENO, buf, len);
                    // 发送给客户端
                    send(fd, buf, len, 0);
                }
                if(len == 0)
                {
                    printf("客户端断开了连接\n");
                    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    if(ret == -1)
                    {
                        perror("epoll_ctl - del error");
                        exit(1);
                    }
                    close(fd);
                }
                else if(len == -1)
                {
                    if(errno == EAGAIN)
                    {
                        printf("缓冲区数据已经读完\n");
                    }
                    else
                    {
                        printf("recv error----\n");
                        exit(1);
                    }
                }
            }
        }
    }
 
    close(lfd);
    return 0;
}
