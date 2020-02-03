#include "Thread.h"
#include <iostream>
#include <unistd.h>
#include <memory>
int a=0;

void func()
{
    std::cout<<"ChildThread is run!\n";
    sleep(3);
    std::cout<<"ChildThread is finished!\n";
}
//由于这里创建thread没有加锁，所以显示有一些凌乱
int main()
{
    std::cout<<"FatherThread is run!\n";
    Thread::ThreadCallback Callback(func);

    std::shared_ptr<Thread> thread[10];
    for (int i=0; i<10; i++)
    {
        std::shared_ptr<Thread> temp(new Thread(Callback));
        thread[i]=temp;
    }
    std::cout<<"create finished!\n";
    for (int i=0; i<10; i++)
    {
        thread[i]->start();
    }
    for (int i=0; i<10; i++)
    {
        thread[i]->join();
    }
    std::cout<<"FatherThread is finished!\n";
}