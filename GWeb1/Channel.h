#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;

class Channel
{
public:
    Channel(){};
    Channel(int Channelfd):fd(Channelfd){};
    /*处理文件描述符和事件的函数*/
    int GetFd(){return fd;}
    void SetFd(int filefd){fd = filefd;}
    uint32_t GetEvent(){return events;}
    void SetEvent(uint32_t ev){events=ev;}
    uint32_t GetRevent(){return revents;}
    void SetRevent(uint32_t rev){revents=rev;}

    /*处理回调函数的函数*/
    typedef std::function<void()> CallBack;
    void setReadHandler(const CallBack &readHandler) { readHandler_ = readHandler; }
    void setWriteHandler(const CallBack &writeHandler) {writeHandler_ = writeHandler;} 
    void setErrorHandler(const CallBack &errorHandler) {errorHandler_ = errorHandler;}
    void setConnHandler(const CallBack &connHandler) { connHandler_ = connHandler; }

    void handleEvents();
    
    void handleRead();
    void handleWrite();
    void handleError();
    void handleConn();

private:
    int fd;
    uint32_t events;//关注的事件
    uint32_t revents;//活跃的事件
        
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

};