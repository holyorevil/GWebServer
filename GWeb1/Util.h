#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ctype.h>
#include <string>
namespace sockets
{
int socket_bind_listen(int port=8080);
int SetNodelay(int socketfd);
void handle_for_sigpipe();

ssize_t readn(int fd, void *buff, size_t n);//从fd读n个字节到buff中
ssize_t readn(int fd, std::string &inBuffer, bool &zero);//将fd下的所有可读内容读到inBuffer中，zero如果是true，则fd下没有可读的东西了。
//其实这个zero是为了当检测到读到数据为0，就默认为对端关闭，但是个人觉得从返回值是否为0来判断，就可以了，muduo也只是用返回值来判断
ssize_t readn(int fd, std::string &inBuffer);//同上，只是少了一个输出参数
ssize_t writen(int fd, void *buff, size_t n);//将buff中前n个字节写到fd中
ssize_t writen(int fd, std::string &sbuff);//将sbuff中内容写到fd中
}