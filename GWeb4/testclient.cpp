#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
using namespace std;
 
int main(int argc, const char* argv[])
{
	if (argc<2)//需要传入端口号
	{
		cout<<"input ./a.out port"<<endl;
		exit(1);
	}
	int sfd;//客户端只需要一个负责读写的文字描述符就够了
	int port=atoi(argv[1]);//传入的是char*类型的端口号，转换成int型
        /*定义sockaddr_in结构体*/
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);//将ip转换成int型存入sockaddr_in结构体中


	socklen_t addrlen;
	sfd=socket(AF_INET, SOCK_STREAM, 0);

	int flag = fcntl(sfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(sfd, F_SETFL, flag);

	cout<<"-------------before connect------------------\n";
	int ret = connect(sfd, (struct sockaddr*) &addr, sizeof(addr));
	if (errno==EINPROGRESS)
	{
		cout<<"connecting...\n";
		fd_set write_fd;
		FD_ZERO(&write_fd);
		FD_SET(sfd, &write_fd);
		struct timeval timeout;         
        timeout.tv_sec = 6; 
        timeout.tv_usec = 0;
		ret = select(sfd + 1, NULL, &write_fd, NULL, &timeout);//如果6秒后还没有可写事件产生，就返回
		if(ret < 0)
		{
            perror("select");
            close(sfd);
            exit(-1);
        }
        else if (ret == 0)//表示select阻塞超时                         
        {               
            printf( "connection timeout\n" );                
            close(sfd);
            exit(-1);
        }else
		{
			if (!FD_ISSET(sfd, &write_fd))//sfd仍然不可写
			{
				cout<<"err, no events found!\n";
				close(sfd);
			}
			else
			{
				int err = 0;
				socklen_t elen = sizeof(err);
				ret = getsockopt(sfd, SOL_SOCKET, SO_ERROR, (char *)&err, &elen);
				if(ret < 0)
				{
					perror("getsockopt");
					close(sfd);
					exit(-1);
				}
				if(err != 0)
				{
					printf("connect failed with the error: (%d)%s\n", err, strerror(err));
					close(sfd);
					exit(-1);
				}
				else
				{
					printf("connected.\n");
				}
			}
			

		}	
	}
	else if (ret<0)
	{
		perror("connect error is");
		return 0;
	}

	cout<<"-------------connect finish------------------\n";
	int len;
	while (1)
	{
		char buf[1024];
		char quit[]="quit\n";
		bzero(buf, 1024);
		fgets(buf, sizeof(buf), stdin);//从终端读取字符串
		write(sfd, buf, strlen(buf));
		bzero(buf, 1024);
		while(1)
		{
			//len = recv(sfd, buf, sizeof(buf), 0);
			len = read(sfd, buf, sizeof(buf));
			if (len>=0)
			{
				cout<<buf;
				break;
			}
			else if(errno==EAGAIN|errno==EWOULDBLOCK||errno==EINTR)
			usleep(100);
			else
			{
				perror("read");
				cout<<"error"<<endl;
				exit(1);
			}
		}
		if (len == 0)
			break;
		if (!strcmp(buf, quit))
			break;
	}	
	close(sfd);
}
