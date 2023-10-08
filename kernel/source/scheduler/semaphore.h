#pragma once

#include "arch/i386/timer/time_units.h"
#include "thread.h"
#include <vector>

namespace sched
{

enum semaphore_error
{
    kSemaphoreErrorOk,
    kSemaphoreErrorTimedOut
};

struct semaphore
{
    semaphore(int slots = 2);

    void Wait();
    int Wait(time::millisec_t timeout);
    void Notify();
    bool IsFree() const;
private:
    volatile int m_Counter;
    std::vector<int> m_SuspendedThreads;
};

struct mutex
{
    mutex();
    void Lock();
    int Lock(time::millisec_t timeout);
    void Release();
private:
    bool m_Flag;
    std::vector<int> m_SuspendedThreads;
};

}
