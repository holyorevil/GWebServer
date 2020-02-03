### 一、项目描述
本项目是用C++11编写的Web多线程高性能服务器，提供了HTTP协议中的get，head，post三种请求方式，支持HTTP长连接及管线化请求，并且实现双缓冲异步日志功能。
### 二、项目环境
- OS：Ubuntu 16.04
- Complier: g++ 5.4
### 三、项目说明
本项目是参考了陈硕的muduo高性能服务器框架，以一个本人编写的最简单的多路复用的单线程tcp微型服务器为基础，逐步加入muduo的Reactor模式而来的。
技术要点如下。
- 使用Epoll水平触发的IO多路复用技术，非阻塞IO，使用Reactor模式
- 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
- 使用基于小根堆的定时器关闭超时请求
- 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
- 使用eventfd实现了线程的异步唤醒
- 使用双缓冲区技术实现了简单的异步日志系统
- 为减少内存泄漏的可能，使用智能指针等RAII机制
- 使用状态机解析了HTTP请求,支持管线化
- 支持优雅关闭连接  

项目框架图如下所示：
![框架图](https://github.com/holyorevil/GWebServer/blob/master/pic/model.png)
### 四、项目测试
##### 1、日志测试
日志测试以muduo服务器为对照，测试数据如下：
![日志测试数据表](https://github.com/holyorevil/GWebServer/blob/master/pic/logtest.png)
分析制作成折线图如下：
![日志短连接测试折线图](https://github.com/holyorevil/GWebServer/blob/master/pic/shortlog.png)
![日志长连接测试折线图](https://github.com/holyorevil/GWebServer/blob/master/pic/longlog.png)
##### 2、压力测试
压力测试使用的是开源压测工具webbench，并且以muduo和另一个类muduo开源网络库WebServer(https://github.com/linyacool/WebServer)作为参考对象：
![压测数据表](https://github.com/holyorevil/GWebServer/blob/master/pic/pressuretest.png)
![压测柱状图](https://github.com/holyorevil/GWebServer/blob/master/pic/pressurepic.png)

