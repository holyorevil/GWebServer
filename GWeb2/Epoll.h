#pragma once
#include <vector>
#include <memory>
#include "Channel.h"
#include "Timer.h"
/*epoll函数的封装，并且让epoll队列中的元素由文件描述符替换成了封装好的Channel*/
typedef std::shared_ptr<Channel> ChannelPtr;

class HttpConnect;
class HttpListen;
class TimerQueue;
class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_add(ChannelPtr channel, int timeout);
    int epoll_mod(ChannelPtr channel, int timeout);//有待完善
    int epoll_del(ChannelPtr channel);
    int poll(std::vector<ChannelPtr> &ActiveChannel);//感觉返回一个vector数组不够高效

    //定时器相关函数
    void AddTimerNode(std::shared_ptr<Channel> request_data, int timeout);
    void UpdateTimerNode(std::shared_ptr<HttpConnect> request_data, int timeout);
    void handleExpired() { timerQueue.handleExpiredEvent(); }
private:
    static const int MAXFDS = 100000;
    int epollfd;
    std::vector<ChannelPtr> ActiveChannel;//活跃Channel列表
    std::shared_ptr<Channel> fd2chan_[MAXFDS];//用来存放挂在epoll队列上的Channel的数组。muduo是用map来存放的
    std::shared_ptr<HttpConnect> fd2http_[MAXFDS];//用来存放在epoll队列上挂的Channel所属的HttpConnect
    //因为在Server每次创建一个临时shared_ptr<HttpConnect>对象，然后挂到epoll队列上时，就会存储在这里，然后Server中的shared_ptr<HttpConnection>
    //就会很快被消灭

    TimerQueue timerQueue;
};