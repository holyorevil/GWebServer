#include "Server.h"


int main(int argc, const char* argv[])
{
    int port = 80;
    if (argc==2)
        port = atoi(argv[1]);
    
    EventLoop eventloop;
    Server server(&eventloop, port);
    server.start();
    return 0;
}