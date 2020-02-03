#pragma once
#include <memory>
#include <unistd.h>
class Channel;
class TimerNode;
class EventLoop;

class HttpListen : public std::enable_shared_from_this<HttpListen>
{
public:
    HttpListen(EventLoop* loop_, int port_);
    ~HttpListen(){::close(socketfd);};
    void setNewConnection(const std::function<void (int)> &newConnection_){newConnection = newConnection_;}
    void Listen();
private:
    EventLoop* loop;
    int socketfd;
    int port;
    std::shared_ptr<Channel> ConnectChannel;
    std::function<void (int)> newConnection;
    void handleRead();

};