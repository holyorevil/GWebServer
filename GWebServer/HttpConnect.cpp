#include "HttpConnect.h"
#include "Util.h"
#include "EventLoop.h"
#include <iostream>
#include <sstream>
#include "base/Logging.h"
static const int DEFAULT_EXPIRED_TIME = 2000;              // ms

HttpConnect::HttpConnect(EventLoop* loop_, int connfd_)
                :loop(loop_),
                connfd(connfd_),
                ConnectChannel(new Channel(loop_, connfd_)),
                keepAlive(false),
                errnum(0)
{
    ConnectChannel->setReadHandler(std::bind(&HttpConnect::handleRead,this));
    ConnectChannel->setErrorHandler(std::bind(&HttpConnect::handleError,this));
    ConnectChannel->SetEvent(EPOLLIN|EPOLLET);
}

void HttpConnect::handleError()
{
    //LOG_INFO<<"连接出现错误, 连接关闭";
    loop->runInLoop(std::bind(&HttpConnect::handleClose,shared_from_this()));
}

void HttpConnect::handleClose()
{
    //LOG_INFO<<"连接关闭";
    std::shared_ptr<HttpConnect> guard(shared_from_this());//为了保证在这个函数结束时，才析构HttpConnect对象
    loop->removeFromPoller(ConnectChannel);
}

void HttpConnect::connectInit()
{
    ConnectChannel->SetUpHttpConnect(shared_from_this());//将Channel与HttpConnect绑定
    loop->addToPoller(ConnectChannel, DEFAULT_EXPIRED_TIME);//设置定时器
    //loop->addToPoller(ConnectChannel, 0);//不设置定时器
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

void HttpConnect::handleRead()//从客户端读取数据以后具体的操作函数在这里编写
{
    /* TODO */
    //tcpEcho();
    http();
}


void HttpConnect::tcpEcho()
{
    ssize_t len;
    // 从客户端读取数据
    len=sockets::readn(connfd, inBuffer);
    if(len == 0)
    {
        //LOG_INFO<<"客户端断开了连接";
        loop->runInLoop(std::bind(&HttpConnect::handleClose,shared_from_this()));
        return;
    }
    else if(len == -1)
    {
        if(errno == EAGAIN)
        {
            //LOG_INFO<<"缓冲区数据已经读完";
            return;
        }
        else
        {
            //LOG_INFO<<"连接出现错误, 连接关闭";
            loop->runInLoop(std::bind(&HttpConnect::handleClose,shared_from_this()));
            return;
        }
    }
    write(STDOUT_FILENO, inBuffer.c_str(), len);
    sockets::writen(connfd, inBuffer);
    loop->updateTimerNode(shared_from_this(), DEFAULT_EXPIRED_TIME);//每次触发读事件以后，都会重置定时器，
    //其实就是将原来的定时节点和HttpConnect对象分离，然后再新建一个定时节点，绑定当前HttpConnect对象，之前的节点在到期以后就删除
}

void HttpConnect::http()
{
    //连接初始化
    outBuffer.clear();
    inBuffer.clear();//如果是长连接，就需要重复使用这两个Buffer
    httpInfo.clear();
    errnum = 0;

    ssize_t len;
    // 从客户端读取数据
    len=sockets::readn(connfd, inBuffer);
    //LOG_INFO<<"请求报文:\n"<<inBuffer;
    if (len<0)
    {
        //LOG_WARN<<"数据读取错误";
        wrongRequest("数据读取错误\n");
        errnum=-1;
    }else if(len == 0)
    {
        //LOG_INFO<<"连接结束";
        errnum=-2;
        httpInfo.state = H_SEND;
    }
    if (httpInfo.state==H_START)
    {
        splitRequestLine();
        if (errnum < 0)
        {
            //LOG_WARN<<"请求行解析错误\n";
            wrongRequest("请求行解析错误\n");
        }
        else httpInfo.state=H_HEADER;
    }
    if (httpInfo.state==H_HEADER)
    {
        splitHeader();
        if (errnum<0)
        {
            //LOG_WARN<<"请求体解析错误\n";
            wrongRequest("请求体解析错误\n");
        }
        else 
        {
            if (httpInfo.method == METHOD_POST)
                httpInfo.state = H_BODY;
            else httpInfo.state = H_ANALYSE;
        }
    }
    if (httpInfo.state == H_BODY)
    {
        //std::cout<<inBuffer;//如果有请求体，就直接打印出来
        httpInfo.state = H_ANALYSE;
    }
    if (httpInfo.state == H_ANALYSE)
    {
        analyseRequest();
        httpInfo.state = H_SEND;
    }
    if (httpInfo.state == H_SEND)        
        loop->runInLoop(std::bind(&HttpConnect::sendResponse, shared_from_this())); 
}

void HttpConnect::splitRequestLine()
{
    int pos = inBuffer.find("\r\n");//返回\r\n在inBuffer的第一次出现的位置的\r前面的那个位置
    if (pos<0)//进入这个判断，说明没有\r\n
    {
        //LOG_WARN<<"格式错误，没有请求行";
        errnum = -3;
        return;
    }
    std::string RequestLine;
    if ((int)inBuffer.size()>pos+2)
    {
        RequestLine = inBuffer.substr(0, pos+2);
        inBuffer=inBuffer.substr(pos+2);
    }else RequestLine.swap(inBuffer);//只有请求行
    std::istringstream split(RequestLine);
    std::string tempMethod,tempPath,tempVersion;
    split>>tempMethod>>tempPath>>tempVersion;//将请求行按空格分割成三个字符串，并将\r\n去除
    if (tempMethod.empty()||tempPath.empty()||tempVersion.empty())
    {
        //LOG_WARN<<"请求行信息不全";
        errnum = -4;
        return;
    }
    int ret = httpInfo.SetMethod(tempMethod);
    if (ret==-1)
    {
        //LOG_WARN<<"请求方法无效";
        errnum = -5;
        return;
    }
    ret = httpInfo.SetVersion(tempVersion);
    if (ret==-1)
    {
        //LOG_WARN<<"请求版本无效";
        errnum = -6;
        return;
    }
    httpInfo.path=tempPath;
    return;
}

void HttpConnect::splitHeader()
{
    int pos = inBuffer.find("\r\n\r\n");
    if (pos<0)
    {
        //LOG_WARN<<"请求体格式错误";
        errnum = -7;
        return;
    }
    std::string header;
    header=inBuffer.substr(0, pos+2);
    if ((int)inBuffer.size()>pos+4)
    {
        inBuffer=inBuffer.substr(pos+4);

    }
    httpInfo.state = H_ANALYSE;
    std::string key,value;
    int index;
    while (((pos = header.find("\r\n"))>0)&&(!header.empty()))
    {
        std::string oneHead = header.substr(0, pos);
        if ((int)header.size()>pos+2)
        {
            header=header.substr(pos+2);
        }
        else header.clear();
        index = oneHead.find(":");
        if (index<0)
        {
            continue;
        }
        key = oneHead.substr(0, index);
        if ((int)oneHead.size()<=index+1)
            continue;
        value = oneHead.substr(index+1);
        sockets::trim(key);
        sockets::trim(value);
        httpInfo.headers[key]=value;
    }
    return;    
}

void HttpConnect::analyseRequest()
{
    std::string response;
    response += "HTTP/1.1 200 OK\r\n";
    if (httpInfo.headers.find("Connection") != httpInfo.headers.end() &&
        (httpInfo.headers["Connection"] == "Keep-Alive" ||
         httpInfo.headers["Connection"] == "keep-alive")) {
      keepAlive = true;
      response += "Connection: Keep-Alive\r\nKeep-Alive: timeout=" +
                std::to_string(DEFAULT_EXPIRED_TIME) + "\r\n";
      //std::cout<<"长连接\n";
    }
    if (httpInfo.path == "/") response += "Content-Length: 11\r\n\r\nHello World";
    else 
    {
        wrongRequest("没有此URI");
        errnum = -8;
        return;
    }
    outBuffer.clear();
    outBuffer = response;  
}

void HttpConnect::wrongRequest(std::string msg)
{
    std::string body_buff, header_buff;
    body_buff += "<html><title>哎~出错了</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += msg;
    body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 400 error\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: LinYa's Web Server\r\n";
    ;
    header_buff += "\r\n";
    outBuffer.clear();
    outBuffer = header_buff + body_buff;

    httpInfo.state = H_SEND;
}

void HttpConnect::sendResponse()
{
    if (!outBuffer.empty())
    {
        //LOG_INFO<<"即将发送以下内容\n"<<outBuffer;
        sockets::writen(connfd, outBuffer);
    }
    seperateTimer();
    if (errnum<0)
    {
        //LOG_INFO<<"解析错误，关闭此次连接";
        loop->runInLoop(std::bind(&HttpConnect::handleClose,shared_from_this()));
    }else if (keepAlive)
    {
        loop->updateTimerNode(shared_from_this(), DEFAULT_EXPIRED_TIME);
    }else
    {
        //LOG_INFO<<"短连接结束，关闭此次连接";
        loop->runInLoop(std::bind(&HttpConnect::handleClose,shared_from_this()));
    }
    
}