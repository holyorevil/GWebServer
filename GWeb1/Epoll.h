#pragma once
#include <vector>
#include <memory>
#include "Channel.h"
typedef std::shared_ptr<Channel> ChannelPtr;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_add(ChannelPtr channel, int timeout);
    int epoll_mod(ChannelPtr channel, int timeout);//有待完善
    int epoll_del(ChannelPtr channel);
    int poll(std::vector<ChannelPtr> &ActiveChannel);//感觉返回一个vector数组不够高效
private:
    static const int MAXFDS = 100000;
    int epollfd;
    std::vector<ChannelPtr> ActiveChannel;//活跃Channel列表
    std::shared_ptr<Channel> fd2chan_[MAXFDS];//用来存放挂在epoll队列上的Channel的数组。muduo是用map来存放的
};