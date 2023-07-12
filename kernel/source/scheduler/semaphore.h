#pragma once

#include "thread.h"
#include <vector>

namespace sched
{

struct semaphore
{
    semaphore(int slots = 2);

    void Wait();
    void Notify();
    bool IsFree() const;
private:
    int m_Counter;
    std::vector<thread_t> m_SuspendedThreads;
};

struct mutex
{
    mutex();
    void Lock();
    void Release();
private:
    bool m_Flag;
    std::vector<thread_t> m_SuspendedThreads;
};

}
