/* $Id: socket.c 1.1 1995/01/01 07:11:14 cthuang Exp $
 *
 * This module has been modified by Radim Kolar for OS/2 emx
 */

/***********************************************************************
  module:       socket.c
  program:      popclient
  SCCS ID:      @(#)socket.c    1.5  4/1/94
  programmer:   Virginia Tech Computing Center
  compiler:     DEC RISC C compiler (Ultrix 4.1)
  environment:  DEC Ultrix 4.3 
  description:  UNIX sockets code.
 ***********************************************************************/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//开启客户端的函数封装
int Socket(const char *host, int clientPort)//这个host可以是主机的ip地址，也可以是主机域名
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    //如果是主机IP地址，就使用inet_addr填充函数ad.sin_addr
    inaddr = inet_addr(host);//将点分十进制的IP转换成一个长整数型
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else//如果是域名，就使用gethostbyname函数得到ip长整数型
    {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);//大小端转换
    
    sock = socket(AF_INET, SOCK_STREAM, 0);

    
    if (sock < 0)
        return sock;

    // int optval = 1;
    // if(setsockopt(sock, SOL_SOCKET,  SO_REUSEPORT, &optval, sizeof(optval)) == -1)
    //     return -1;
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
    {
        close(sock);
        return -1;
    }

    return sock;
}

