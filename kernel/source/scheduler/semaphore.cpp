#include "semaphore.h"
#include "arch/i386/timer/time_units.h"
#include "scheduler/scheduler.h"
#include "scheduler/thread.h"
#include <cstddef>

[[gnu::always_inline]]
void hlt()
{
    asm volatile("hlt");
}

constexpr static int abs(int v)
{
    return v < 0 ? v * (-1) : v;
}

static void SemaphoreTrigger(void* f)
{
    auto flag = reinterpret_cast<bool*>(f);
    (*flag) = false;
}

sched::semaphore::semaphore(int slots)
    : m_Counter(abs(slots))
{
}

void sched::semaphore::Notify()
{
    m_Counter++;
    if (m_Counter < 0)
    {
        auto ab = abs(m_Counter);
        auto tid = m_SuspendedThreads[ab];
        auto thread = CScheduler::GetThread(tid);
        thread->SetSuspended(false, kThreadSuspendReasonNotSuspended);
    }
}

bool sched::semaphore::IsFree() const
{
    return m_SuspendedThreads.empty();
}

void sched::semaphore::Wait()
{
    m_Counter--;
    if (m_Counter < 0)
    {
        CThread*& s = CScheduler::GetCurrentThread();
        s->SetSuspended(true, kThreadSuspendReasonWaiting);
        m_SuspendedThreads.push_back(CScheduler::GetCurrentThreadId());

        while(true)
        {
            hlt();
        }
    }
}

int sched::semaphore::Wait(time::millisec_t timeout)
{
    m_Counter--;
    if (m_Counter < 0)
    {
        bool* flag = new bool;
        AsyncTimer(timeout, SemaphoreTrigger, flag);

        while (m_Counter < 0 && *flag)
        {
            hlt();
        }

        if (!flag)
        {
            delete flag;
            return kSemaphoreErrorTimedOut;
        }

        delete flag;
    }

    return kSemaphoreErrorOk;
}

sched::mutex::mutex()
    : m_Flag(true)
{
}

void sched::mutex::Release()
{
    if (!m_Flag)
    {
        m_Flag = true;
        CThread* th;
        
        for (const int& i : m_SuspendedThreads)
        {
            th = CScheduler::GetThread(i);
            th->SetSuspended(false, kThreadSuspendReasonNotSuspended);
        }
    }
}

void sched::mutex::Lock()
{
    if (m_Flag)
    {
        m_Flag = false;
        return;
    }

    auto& t = CScheduler::GetCurrentThread();
    m_SuspendedThreads.push_back(CScheduler::GetCurrentThreadId());
    t->SetSuspended(true, kThreadSuspendReasonWaiting);

    while (true)
    {
        hlt();
    }
}

int sched::mutex::Lock(time::millisec_t timeout)
{
    if (m_Flag)
    {
        m_Flag = false;
        return kSemaphoreErrorOk;
    }

    bool* flag = new bool(true);
    AsyncTimer(timeout, SemaphoreTrigger, flag);
    while (!m_Flag && flag)
    {
        hlt();
    }
    
    if (!flag)
    {
        delete flag;
        return kSemaphoreErrorTimedOut;
    }

    delete flag;
    return kSemaphoreErrorOk;
}
