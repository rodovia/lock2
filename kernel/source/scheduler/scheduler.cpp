#include "scheduler.h"
#include "alloc/physical.h"
#include "arch/i386/cpu/idt.h"
#include "scheduler/thread.h"
#include "arch/i386/apic.h"
#include "terminal.h"
#include <algorithm>

#define EnterCriticalZone() asm volatile("cli")
#define LeaveCriticalZone() asm volatile("sti")

extern "C" void SchSwitchTaskKernel(full_register_state* state);

sched::CScheduler::CScheduler()
    : m_Enabled(false),
      m_Quantum(0),
      m_Index(0)
{
    CThread* thr = new CThread(
        [](void*) 
        { 
            while (true) 
            { 
                asm volatile("hlt");
            } 
        }, nullptr, kThreadKernelMode);
    this->AddThread(thr);
}

void sched::CScheduler::Think(full_register_state regState)
{
    EnterCriticalZone();
    CScheduler* thiz = &CScheduler::GetInstance();
    thiz->ThinkDeep(regState);
    LeaveCriticalZone();
}

void sched::CScheduler::Enable()
{
    EnterCriticalZone();
    m_Enabled = true;
    LeaveCriticalZone();
}

void sched::CScheduler::ThinkDeep(full_register_state regState)
{
    if (!m_Enabled)
    {
        return;
    }
    this->ThinkSuspendedThreads();
    if (m_ThreadCount == 0)
    {
        return;
    }

    if (m_Quantum > 0)
    {
        m_Quantum--;
        return;
    }

    auto& current = m_Threads[m_Index];
    current->SaveState(regState);
    m_Index++;
    if (m_Index >= m_ThreadCount)
    {
        m_Index = 0;
    }
    
    current = m_Threads.at(m_Index);
    if (current == nullptr)
    {
        return;
    }

    m_Quantum = 10;
    acpi::local::EndOfInterrupt();
    SchSwitchTaskKernel(current->m_RegState);
}

void sched::CScheduler::AddThread(CThread* thread)
{
    EnterCriticalZone();
    m_Threads.push_back(thread);
    m_ThreadCount++;
    LeaveCriticalZone();
}

size_t sched::CScheduler::GetThreadCount() const
{
    return m_ThreadCount;
}

void sched::CScheduler::RemoveThread(CThread* thread)
{
    std::erase(m_Threads, thread);
}

sched::CThread*& sched::CScheduler::GetCurrentThread()
{
    auto& thiz = GetInstance();
    return thiz.m_Threads[GetCurrentThreadId()];
}

int sched::CScheduler::GetCurrentThreadId()
{
    auto& thiz = GetInstance();
    return thiz.m_Index;
}

sched::CThread*& sched::CScheduler::GetThread(thread_t id)
{
    auto& thiz = GetInstance();
    return thiz.m_Threads[id];
}

void sched::CScheduler::AddSuspendedThread(CThread *thread, time::millisec_t ticks)
{
    EnterCriticalZone();
    auto t = new thread_sleep_info{thread, ticks};
    m_SuspendedThreads.push_back(t);
    LeaveCriticalZone();
}

void sched::CScheduler::RemoveSuspendedThread(thread_sleep_info* i)
{
    std::erase(m_SuspendedThreads, i);
}

void sched::CScheduler::ThinkSuspendedThreads()
{
    for (auto& i : m_SuspendedThreads)
    {
        i->RemainingTicks -= 1;

        if (i->RemainingTicks <= 0)
        {
            this->RemoveSuspendedThread(i);
        }
    }

}

void sched::CScheduler::YieldThreadTime()
{
    m_Quantum = 0;
}
