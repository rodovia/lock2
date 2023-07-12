#include "semaphore.h"
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
            asm volatile("hlt");
        }
    }
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
