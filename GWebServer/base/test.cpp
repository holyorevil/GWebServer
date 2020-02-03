#include "Logging.h"
#include "AsyncLogging.h"
int main()
{
    AsyncLogging log("./log/test");
    Logger::setOutputFunc(std::bind(&AsyncLogging::append, &log, std::placeholders::_1, std::placeholders::_2));
    log.start();
    LOG_INFO<<"hello world!";
    LOG_WARN<<"hello world!";
    LOG_ERROR<<"hello world!";

}