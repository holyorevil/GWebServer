#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <boost/any.hpp>
/*Channel其实是用来封装epoll队列上每一个文件描述符的类，封装了文件描述符，被关注的事件，以及处理事件的函数*/
class EventLoop;
class HttpConnect;

class Channel
{
public:
    Channel(EventLoop* loop_):loop(loop_),
                              events(0),
                              revents(0),
                              tied(false){}
    Channel(EventLoop* loop_, int Channelfd_):fd(Channelfd_),
                                              loop(loop_),
                                              events(0),
                                              revents(0),
                                              tied(false){}
    ~Channel(){};

    //操作UpHttpConnect变量
    void SetUpHttpConnect(std::shared_ptr<HttpConnect> holder) 
    {
        UpHttpConnect=holder;
        tied=true;
        return;
    }
    std::shared_ptr<HttpConnect> GetUpHttpConnect() 
    {
        std::shared_ptr<HttpConnect> ret(UpHttpConnect.lock());
        return ret;
    }//返回值是一个shared_ptr是无奈之举
    //操作UpClass变量
    //void SetUpClass(std::shared_ptr<void> tie){UpClass=tie;};

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
    EventLoop* loop;//这个Channel所属的epoll队列，但是暂时是没有用处的，在muduo中存储EventLoop指针是为了在epoll队列中修改或者移除Channel时，确认这个Channel是属于这个队列的
    uint32_t events;//关注的事件
    uint32_t revents;//活跃的事件
    //注意，我这里没有旧的关注的事件，是因为我把WebServer中的epoll类中epoll_mod函数的功能拆分成了两个函数，UpdateTimerNode和epoll_mod


    //std::weak_ptr<HttpConnect> UpClass;//因为一般会对Channel再进一步封装，比如WebServer中的HttpData类，muduo中的Acceptor，TcpConnection
    //所以这个变量就是用来存放上层类对象的。在muduo中，这个变量主要作用是通过提升为shared_ptr指针，检测上层类对象是否还存在，可以参见Channel.cc中handleEvent函数
    //而在Webserver中，只是为了将HttpConnect存放到Epoll类中的fd2http_数组中，我认为需要改进。！！！
    //但是注意，以muduo来说，muduo的Channel类中的weak_ptr<void> tie_也只会绑定TcpConnection上层类，而其他类没有绑定
    //可以考虑其他上层类有Acceptor，还有就是EventLoop中有WakeUpChannel类，这些类基本都是从程序开始创建一直到程序结束才会释放的。
    //所以不需要。这里我考虑较繁琐了，认为所有的Channel都需要判断上层类是否还存在。所以在muduo中多了一个标志位tied_,就是为了区分这个
    //Channel是否绑定了上层类 
    
    
    //这是对上面所说的改进
    std::weak_ptr<HttpConnect> UpHttpConnect;//只有在上层封装类是HttpConnect时，才会使用，在Epoll::epoll_add函数中会使用，在Channel::handleEvents函数中也会使用
    bool tied;
    //这是考虑繁琐的印记//std::weak_ptr<void> UpClass;//这个是所有封装了Channel的上层类，现在主要是HttpConnect和HttpClient两个类，都会使用，为了在执行事件发生函数时，检测上层类是否还存在。

    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

};