#include "AsyncLogging.h"
#include "Logging.h"

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <chrono>

off_t kRollSize = 500*1000*1000;

AsyncLogging* g_asyncLog = NULL;

void bench(bool longLog)
{
  int cnt = 0;
  const int kBatch = 1000;
  std::string empty = " ";
  std::string longStr(3000, 'X');
  longStr += " ";

  for (int t = 0; t < 30; ++t)
  {
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kBatch; ++i)
    {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    anto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout<<"elapsed time: " << elapsed_seconds.count() << endl;
    struct timespec ts = { 0, 500*1000*1000 };
    nanosleep(&ts, NULL);
  }
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

  char name[256] = { '\0' };
  strncpy(name, argv[0], sizeof name - 1);
  g_asyncLog(::basename(name), kRollSize);
  g_asyncLog->start();

  bool longLog = argc > 1;
  bench(longLog);
}