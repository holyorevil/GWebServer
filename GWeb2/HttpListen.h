#pragma once
#include <memory>
#include <unistd.h>
class Channel;
class TimerNode;
class EventLoop;

class HttpListen:public std::enable_shared_from_this<HttpListen>
{
public:
    HttpListen(EventLoop* loop_, int port_);
    ~HttpListen(){::close(socketfd);};

    void Listen();
private:
    EventLoop* loop;
    int socketfd;
    int port;
    std::shared_ptr<Channel> ConnectChannel;

    void handleRead();

};