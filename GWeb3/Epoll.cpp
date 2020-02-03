#include "Epoll.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "HttpConnect.h"

const int EPOLLWAIT_TIME = 10000;//每次epoll_wait设置的超时时间（10s）

Epoll::Epoll():epollfd(::epoll_create1(EPOLL_CLOEXEC)), ActiveChannel(2048)
{
    if (epollfd<0)
    {
        ::abort();
    }
}

Epoll::~Epoll()
{
    ::close(epollfd);
}

int Epoll::epoll_add(ChannelPtr channel, int timeout)
{
    if (timeout>0)
    {
        AddTimerNode(channel, timeout);
    }
    struct epoll_event ev;
    int fd = channel->GetFd();
    ev.data.fd=fd;
    ev.events=channel->GetEvent();
    fd2chan_[fd] = channel;
    fd2http_[fd] = channel->GetUpHttpConnect();
    if (::epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)<0)
    {
        int err=errno;
        std::cout<<err<<std::endl;
        perror("::epoll_ctl error is: ");
        fd2chan_[fd].reset();
        fd2http_[fd].reset();
        return -1;
    }
    std::cout<<"epoll_add success\n";
    return 0;
}

int Epoll::epoll_mod(ChannelPtr channel, int timeout)
{
    if (timeout>0)
    {
        AddTimerNode(channel, timeout);
    }
    struct epoll_event ev;
    int fd = channel->GetFd();
    ev.data.fd=fd;
    ev.events=channel->GetEvent();
    if (::epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev)<0)
    {
        int err=errno;
        std::cout<<err<<std::endl;
        perror("::epoll_ctl error is: ");
        fd2chan_[fd].reset();
        fd2http_[fd].reset();
        return -1;
    }
    std::cout<<"epoll_mod success\n";
    return 0;
}

int Epoll::epoll_del(ChannelPtr channel)
{
    int fd = channel->GetFd();
    fd2chan_[fd].reset();
    if (::epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0)<0)
    {
        fd2chan_[fd].reset();
        fd2http_[fd].reset();
        return -1;
    }
    fd2chan_[fd].reset();//释放fd2chan_[fd]中的shared_ptr指针。
    fd2http_[fd].reset();
    return 0;
}

int Epoll::poll(std::vector<ChannelPtr> &ActiveChannel)
{
    std::vector<struct epoll_event> ActiveEvent(1024);
    int ret = ::epoll_wait(epollfd, &*ActiveEvent.begin(), ActiveEvent.size(), EPOLLWAIT_TIME);
    if (ret < 0)
    {
        perror("epoll wait error");
        return -1;
    }
    ActiveChannel.clear();
    for (int i=0; i < ret; i++)
    {
        int fd = ActiveEvent[i].data.fd;
        ChannelPtr temp = fd2chan_[fd];//利用fd2chan_来初始化临时的ChannelPtr
        temp->SetRevent(ActiveEvent[i].events);
        temp->SetEvent(0);
        ActiveChannel.push_back(temp);
    }
    return 0;
}

void Epoll::AddTimerNode(std::shared_ptr<Channel> request_data, int timeout)
{
    std::shared_ptr<HttpConnect> temp=request_data->GetUpHttpConnect();
    if (temp) timerQueue.addTimer(temp, timeout);
}

void Epoll::UpdateTimerNode(std::shared_ptr<HttpConnect> request_data, int timeout)
{
    request_data->seperateTimer();
    if (timeout>0)
    {
        AddTimerNode(request_data->GetChannel(), timeout);
    }
}