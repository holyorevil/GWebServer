#pragma once
#include <map>

//每个请求经过解析以后，就会把对应的信息分门别类地放到这个类的对象中
enum Method
{
    METHOD_INVALID, METHOD_POST, METHOD_GET, METHOD_HEAD
};
enum Version
{
    HTTP_UNKNOWN, HTTP_10, HTTP_11
};
enum ParseState
{
    H_START,
    H_HEADER,
    H_BODY,
    H_ANALYSE,
    H_SEND
};

struct HttpRequestInfo
{
    HttpRequestInfo():state(H_START),method(METHOD_INVALID),version(HTTP_UNKNOWN){}
    void clear()
    {
        state = H_START;
        method = METHOD_INVALID;
        version = HTTP_UNKNOWN;
        path.clear();
        headers.clear();
    }
    int SetMethod(std::string &str)
    {
        if (str == "GET")
            method = METHOD_GET;
        else if(str == "POST")
            method = METHOD_POST;
        else if(str == "HEAD")
            method = METHOD_HEAD;
        else {
            method = METHOD_INVALID;
            return -1;
        }
        return 0;
    }
    int SetVersion(std::string &str)
    {
        if (str == "HTTP/1.1")
            version = HTTP_11;
        else if (str == "HTTP/1.0")
            version = HTTP_10;
        else {
            version = HTTP_UNKNOWN;
            return -1;
        }
        return 0;
    }
    ParseState state;//请求体解析状态标志
    Method method;//请求方法
    Version version;//http版本
    std::string path;//uri
    std::map<std::string, std::string> headers;//请求头处理后键值放第一个，实值放第二个
};