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
压力测试使用的是开源压测工具webbench，并且以muduo和另一个类muduo开源网络库WebServer(https://github.com/linyacool/WebServer) 作为参考对象：

![压测数据表](https://github.com/holyorevil/GWebServer/blob/master/pic/pressuretest.png)

![压测柱状图](https://github.com/holyorevil/GWebServer/blob/master/pic/pressurepic.png)
### 五、项目版本
##### 1、版本一
这个版本是最简单的版本，在我学习epoll函数时，写过一个单线程的echotcp服务器，我将这个程序按照muduo的结构改写成C++版本的，虽然很简单，但是通过这个版本，对于muduo中每个类的理解加深了许多。
##### 2、版本二
这一版本中，新加入定时器功能，定时器主要用于当长连接长时间没有请求时，到时就会自动断开。并且对Channel类进行了多一步的封装了，形成了HttpConnect类和HttpListen类，但是这两个类是面向Channel类，而非基于Channel类，因为这两个类都是把Channel类对象作为自己的成员对象，而不是自己的父类。
##### 3、版本三
第三版本中，主要加入了EventLoop多线程，每接受到一个新的请求连接，就将这个文件描述符放到EventLoopThreadPool中，并且也添加了一些基础类，比如锁，条件变量，带锁计数器等，现在这个库已经是一个比较完整的多线程Tcp回显服务器了
##### 4、版本四
第四版本中，首先加入了HTTP解析功能，能够解析GET，POST，HEAD方法，并且能够根据客户端发来的请求自动判断并实现长短连接。然后加入了异步日志库，并对异步日志库进行测试，性能和muduo的异步日志库进行对比，发现效果要优于muduo异步日志库。
##### 5、版本五
第五版本中，主要是用WebBench进行了压测，在测试中发现一些小bug，比如定时器分离忘记加了，并更改
并将所有挂在epoll上的文件描述符都改成了上边沿触发，之前socket描述符是设置的水平触发。对于这个网络库暂时先更新到这里。
后面还有值得更新的地方：
①首先整个库缺少一个IO缓冲区，我的库中都是使用的string作为缓冲的
②负载均衡只是采用很简单的轮寻的方式，可以采用更加先进的负载均衡方法
③可以实现服务器缓存，加速性能






