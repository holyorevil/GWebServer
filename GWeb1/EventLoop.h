#include "Epoll.h"

class EventLoop
{
public:
    EventLoop();
    void loop();
    //虽然这里多了一层封装，但是由于是在类内定义的，所以是内联函数，其实在EventLoop中的函数封装并没有什么用处
    //主要是如果只使用一层封装，我只想到两种改动，一种是将poller改成共有变量，显然不好，暴露了成员变量，
    //还有一种是这些函数不在Epoll类中定义，而是在EventLoop中定义，这也不好，类的分工就不明确了。所以为了结构的清晰，就选择了牺牲性能的方法
    void removeFromPoller(std::shared_ptr<Channel> channel) {
        poller->epoll_del(channel);
    }
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller->epoll_mod(channel, timeout);
    }
    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller->epoll_add(channel, timeout);
    }
private:
    std::shared_ptr<Epoll> poller;
};