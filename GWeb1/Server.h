#include "EventLoop.h"

class Server
{
public:
    Server(EventLoop* loop_, int port_=80);
    void start();
private:
    EventLoop* loop;//个人理解这里用普通指针而不是智能指针，因为这个Server类是一直随着程序的存在而存在的，
                         //不会在程序结束之前析构的，所以不需要使用智能指针
    int port;
    std::shared_ptr<Channel> SocketChannel;
    int socketfd;
};