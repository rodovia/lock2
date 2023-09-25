#pragma once

#include "arch/i386/timer/time_units.h"
#include "scheduler/thread.h"
#include <vector>

namespace sched
{

struct thread_sleep_info
{
    CThread* Thread;
    time::millisec_t RemainingTicks;
};

class CScheduler
{
public:
    CScheduler();
    CScheduler(CScheduler&&) = delete;

    static CScheduler& GetInstance()
    {
        static CScheduler s;
        return s;
    }

    static CThread*& GetCurrentThread();
    static int GetCurrentThreadId();
    static void Think(full_register_state regState);
    static CThread*& GetThread(thread_t id);
    static CThread* GetSuspendedThread(thread_t id);
    void YieldThreadTime();
    void AddSuspendedThread(CThread* thread, time::millisec_t ticks);
    void RemoveSuspendedThread(thread_t thread);
    void AddThread(CThread* thread);
    void RemoveThread(CThread* thread);
    void SetEnabled(bool enabled);
    size_t GetThreadCount() const;

private:
    void ThinkDeep(full_register_state regState);
    void ThinkSuspendedThreads();

    volatile bool m_Enabled;
    std::vector<thread_sleep_info*> m_SuspendedThreads;
    std::vector<CThread*> m_Threads;
    volatile uint32_t m_ThreadCount;
    volatile uint32_t m_Quantum;
    volatile uint32_t m_Index;
};

}
