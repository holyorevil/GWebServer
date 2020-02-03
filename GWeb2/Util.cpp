#include "Util.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#define MAX_BUFF 4096
/*工具性函数的写在这个文件中*/
int sockets::socket_bind(int port)
{
    /*排除错误端口号*/
    if (port<0 ||port >65535)
    {
        return -1;
    }
    /*创建套接字*/
    int socketfd = 0;//用于监听的文件描述符
    socketfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
    //socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        return -2;
    }

    /*绑定套接字*/
    struct sockaddr_in addr;
    socklen_t addr_len=sizeof(addr);
    memset(&addr, 0, addr_len);
    addr.sin_family = AF_INET;                   // 设置地址族，为IPv4地址 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 监听本机所有的IP，所以本地IP127.0.0.1和inet地址都可以被监听到
    addr.sin_port = htons(port);            // 设置端口 
    if (::bind(socketfd, (struct sockaddr*)&addr, addr_len)<0)
    {
        perror("accept error");
        ::close(socketfd);
        return -3;
    }
    return socketfd;
}

int sockets::Listen(int socketfd, int num)
{
    if (::listen(socketfd, num))
    {
        perror("listen error");
        ::close(socketfd);
        return -1;
    }
    return 0;
}

int sockets::SetNodelay(int socketfd)
{
    int option=1;
    return (::setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option)));
}

void sockets::handle_for_sigpipe() 
{//忽略SIGPIPE信号的函数
//客户端程序向服务器端程序发送了消息，然后关闭客户端，服务器端返回消息的时候就会收到内核给的SIGPIPE信号。
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) return;
}

ssize_t sockets::readn(int fd, void *buff, size_t n) {
  size_t nleft = n;
  ssize_t nread = 0;
  ssize_t readSum = 0;
  char *ptr = (char *)buff;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;
      else if (errno == EAGAIN) {
        return readSum;
      } else {
        return -1;
      }
    } else if (nread == 0)
      break;
    readSum += nread;
    nleft -= nread;
    ptr += nread;
  }
  return readSum;
}

ssize_t sockets::readn(int fd, std::string &inBuffer, bool &zero)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true) {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
            if (errno == EINTR)
            continue;
            else if (errno == EAGAIN) {
            return readSum;
            } else {
            perror("read error");
            return -1;
            }
        } else if (nread == 0) {
            zero = true;
            break;
    }
    readSum += nread;
    inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}


ssize_t sockets::readn(int fd, std::string &inBuffer) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true) {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
        if (errno == EINTR)
            continue;
        else if (errno == EAGAIN) {
            return readSum;
        } else {
            perror("read error");
            return -1;
        }
        } else if (nread == 0) {
        break;
        }
        readSum += nread;
        inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}

ssize_t sockets::writen(int fd, void *buff, size_t n) {
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char *)buff;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
        if (nwritten < 0) {
            if (errno == EINTR) {
            nwritten = 0;
            continue;
            } else if (errno == EAGAIN) {
            return writeSum;
            } else
            return -1;
        }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t sockets::writen(int fd, std::string &sbuff) {
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
        if (nwritten < 0) {
            if (errno == EINTR) {
            nwritten = 0;
            continue;
            } else if (errno == EAGAIN)
            break;
            else
            return -1;
        }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}