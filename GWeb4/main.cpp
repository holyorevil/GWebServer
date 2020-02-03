#include "Server.h"


int main(int argc, const char* argv[])
{
    int port = 8080;
    if (argc==2)
        port = atoi(argv[1]);
    
    EventLoop mainLoop;
    Server server(&mainLoop, 0, port);
    server.start();
    mainLoop.loop();//这是为了开启监听文件描述符的epoll队列
    return 0;
}