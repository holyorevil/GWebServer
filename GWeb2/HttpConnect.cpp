#include "HttpConnect.h"
#include "Util.h"
#include "EventLoop.h"
static const int DEFAULT_EXPIRED_TIME = 2000;              // ms

HttpConnect::HttpConnect(EventLoop* loop_, int connfd_)
                :loop(loop_),
                connfd(connfd_),
                ConnectChannel(new Channel(loop_, connfd_))
{
    ConnectChannel->setReadHandler(std::bind(&HttpConnect::handleRead,this));
    ConnectChannel->setErrorHandler(std::bind(&HttpConnect::handleError,this));
    ConnectChannel->SetEvent(EPOLLIN|EPOLLET);
}

void HttpConnect::handleRead()
{
    std::string buf;
    //bool zero=false;
    ssize_t len;
    // 从客户端读取数据
    len=sockets::readn(connfd, buf);
    if(len == 0)
    {
        printf("客户端断开了连接\n");
        handleClose();
        return;
    }
    else if(len == -1)
    {
        if(errno == EAGAIN)
        {
            printf("缓冲区数据已经读完\n");
            return;
        }
        else
        {
            printf("recv error----\n");
            handleClose();
            return;
        }
    }
    write(STDOUT_FILENO, buf.c_str(), len);
    sockets::writen(connfd, buf);
    loop->updateTimerNode(shared_from_this(), DEFAULT_EXPIRED_TIME);//每次触发读事件以后，都会重置定时器，
    //其实就是将原来的定时节点和HttpConnect对象分离，然后再新建一个定时节点，绑定当前HttpConnect对象，之前的节点在到期以后就删除
}
void HttpConnect::handleError()
{
    printf("连接出现错误\n");
    handleClose();
}

void HttpConnect::handleClose()
{
    std::shared_ptr<HttpConnect> guard(shared_from_this());//为了保证在这个函数结束时，才析构HttpConnect对象
    loop->removeFromPoller(ConnectChannel);
}

void HttpConnect::seperateTimer()
{
    std::shared_ptr<TimerNode> temp(timerNode.lock());
    if(temp)
    {
        temp->clearReq();
        timerNode.reset();
    }
}