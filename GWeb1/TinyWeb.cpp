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

/*第一版本就是将下面这个程序改写成由muduo基本类构成的项目，同样是单线程，没有计时器，长连接，以下标注了下面程序哪一段属于哪一个类中*/
int main(int argc, const char* argv[])
{
    if(argc < 2)
    {
        printf("eg: ./a.out port\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;                           /*21-45行 在Server类的构造函数中*//*21-37行 都属于Util文件中的socket_bind_listen函数*/                                               
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
    int epfd = epoll_create(2000);                          /*43行 在Eventloop类的构造函数中的Epoll类的构造函数*/

    struct epoll_event ev;                                  /*45-139行 在Server类的start函数中*//*45-49行 在Server类的start函数中前三行，分别调用了Channel类的SetEvent，setReadHandler以及EventLoop类的addToPoller函数*/
    // 设置边沿触发
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
 
    struct epoll_event all[2000];
    while(1)                                                /*52-135行 在Eventloop类的loop函数中*/
    {
        // 使用epoll通知内核fd 文件IO检测
        int ret = epoll_wait(epfd, all, sizeof(all)/sizeof(all[0]), -1);    /*55行 在Epoll类中poll函数*/
        printf("================== epoll_wait =============\n");
 
        // 遍历all数组中的前ret个元素
        for(int i=0; i<ret; ++i)
        {
            int fd = all[i].data.fd;
            // 判断是否有新连接
            if(fd == lfd)                                                   /*63-90行 在文件描述符为socket的Channel中的读事件回调函数函数*/
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
                struct epoll_event temp;                                    /*78-81行 在Channel中，创建一个Channel并初始化*/
                // 设置边沿触发
                temp.events = EPOLLIN | EPOLLET;
                temp.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &temp);                 /*82行 在Epoll类中epoll_add函数*/
                
                // 打印客户端信息
                char ip[64] = {0};
                printf("New Client IP: %s, Port: %d\n",
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
                    ntohs(client_addr.sin_port));
                
            }
            else                                                            /*91-133 在文件描述符为connfd的Channel中的读事件回调函数中*/
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
                    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);                 /*113行 在Epoll类中epoll_del函数中*/
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
