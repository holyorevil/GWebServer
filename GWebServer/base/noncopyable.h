#pragma once

class noncopyable
{
protected:
    noncopyable()=default;
    ~noncopyable()=default;
private:
    noncopyable(const noncopyable&)=delete;
    const noncopyable operator=(const noncopyable&)=delete;//利用C++11新特性，让构造拷贝函数和赋值运算符函数不能被调用
};