#include "Channel.h"
void Channel::handleEvents() {
  if (tied)//如果上层封装是HttpConnect类
  {
    if (!UpHttpConnect.lock())//并且该类对象不存在了，就不执行下面的函数
    return;
  }

  if ((revents & EPOLLHUP) && !(revents & EPOLLIN)) {
    events = 0;
    return;
  }
  if (revents & EPOLLERR) {
    if (errorHandler_) errorHandler_();
    events = 0;
    return;
  }
  if (revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    handleRead();
  }
  if (revents & EPOLLOUT) {
    handleWrite();
  }
  handleConn();

}

void Channel::handleRead() {
  if (readHandler_) {
    readHandler_();
  }
}

void Channel::handleWrite() {
  if (writeHandler_) {
    writeHandler_();
  }
}

void Channel::handleConn() {
  if (connHandler_) {
    connHandler_();
  }
}