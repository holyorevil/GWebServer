#include <unistd.h>
#include <memory>
#include <vector>
#include <queue>
class HttpConnect;

class TimerNode
{
public:
    TimerNode(std::shared_ptr<HttpConnect> httpPtr_, int timeout);
    TimerNode(TimerNode &copy);
    ~TimerNode();
    //对私有变量存取的函数
    void update(int timeout);//更新到期时间
    bool isValid();//查看这个计时节点是否到期
    void clearReq();//将这个计时节点清空
    void setDeleted() { deleted = true; }
    bool isDeleted() const { return deleted; }
    size_t getExpTime() const { return expiredTime; }

private:
    bool deleted;
    size_t expiredTime;//到期时间，单位是毫秒
    std::shared_ptr<HttpConnect> httpPtr;
};

struct TimerCmp {
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const {
    return a->getExpTime() > b->getExpTime();
  }
};

class TimerQueue {
 public:
  void addTimer(std::shared_ptr<HttpConnect> SPHttpData, int timeout);
  void handleExpiredEvent();

 private:
  typedef std::shared_ptr<TimerNode> TimerNodePtr;
  std::priority_queue<TimerNodePtr, std::vector<TimerNodePtr>, TimerCmp>//本来priority_queue这边要求的一个参数就是类或者结构体，只是这个结构体中需要重载有两个参数的括号运算符
      timerNodeQueue;
};