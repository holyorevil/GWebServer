#pragma once
#include "Thread.h"

namespace CurrentThread
{
    extern __thread pid_t ThreadPid;
    pid_t getpid();
}