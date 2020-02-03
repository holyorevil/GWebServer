#include "EventLoop.h"
#include <iostream>
EventLoop::EventLoop():poller(new Epoll){};

void EventLoop::loop()
{
    std::vector<ChannelPtr> ActiveEvent;
    while (1)
    {
        ActiveEvent.clear();
        int ret = poller->poll(ActiveEvent);
        if (ret < 0)
        {
            std::cout<<"poll error\n";
            break;
        }
        /*事件处理流程*/
        for (size_t i=0; i<ActiveEvent.size(); i++)
        {
            ActiveEvent[i]->handleEvents();  
        }            
    }
}