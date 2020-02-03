#include "../AsyncLogging.h"
#include "../Logging.h"
#include "Timestamp.h"

#include <stdio.h>
#include <sys/resource.h>

int kRollSize = 500*1000*1000;

AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}

void bench(bool longLog)
{
  Logger::setOutputFunc(std::bind(&AsyncLogging::append, g_asyncLog, std::placeholders::_1, std::placeholders::_2));
  int cnt = 0;
  const int kBatch = 800;
  std::string empty = " ";
  std::string longStr(3000, 'X');
  longStr += " ";
  muduo::Timestamp start = muduo::Timestamp::now();
  for (int t = 0; t < 50; ++t)
  {
    //muduo::Timestamp start = muduo::Timestamp::now();
    for (int i = 0; i < kBatch; ++i)
    {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    //muduo::Timestamp end = muduo::Timestamp::now();
    //printf("%f\n", timeDifference(end, start));//以微秒为单位
  }
  muduo::Timestamp end = muduo::Timestamp::now();
  printf("%f\n", timeDifference(end, start));//以微秒为单位
}

int main(int argc, char* argv[])
{
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000*1024*1024;
    rlimit rl = { 2*kOneGB, 2*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  char name[256];
  strncpy(name, argv[0], 256);
  AsyncLogging log(::basename(name));
  log.start();
  g_asyncLog = &log;

  bool longLog = argc > 1;
  bench(longLog);
}
