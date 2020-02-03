#include "Channel.h"
void Channel::handleEvents() {
  if (UpClass.lock())//只有当上层类对象存在时，才执行下面的回调函数。
  {
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