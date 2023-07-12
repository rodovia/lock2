#include "scheduler.h"
#include "alloc/physical.h"
#include "arch/i386/cpu/idt.h"
#include "scheduler/thread.h"
#include "arch/i386/apic.h"

#define EnterCriticalZone() asm volatile("")
#define LeaveCriticalZone() asm volatile("sti")

extern "C" void SchSwitchTaskKernel(full_register_state* state);

sched::CScheduler::CScheduler()
    : m_Enabled(false),
      m_Quantum(0),
      m_Index(0)
{
}

void sched::CScheduler::Think(full_register_state regState)
{
    EnterCriticalZone();
    CScheduler& thiz = CScheduler::GetInstance();
    thiz.ThinkDeep(regState);
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

    if (m_ThreadCount == 0)
    {
        return;
    }

    if (m_Quantum > 0)
    {
        m_Quantum--;
        return;
    }

    m_Index++;
    if (m_Index > m_ThreadCount)
    {
        m_Index = 0;
    }
    acpi::CApic c;
    c.EndOfInterrupt();
    CThread*& current = m_Threads->GetByIndex(m_Index);

    m_Quantum = 100;
    SchSwitchTaskKernel(current->m_RegState);
}

void sched::CScheduler::AddThread(CThread* thread)
{
    EnterCriticalZone();
    if (m_Threads == nullptr)
    {
        m_Threads = new thread_list;
    }

    thread_list* tmp = m_Threads;
    while (tmp->Next != nullptr)
    {
        tmp = tmp->Next;
    }
    m_ThreadCount++;
    tmp->Next = new thread_list(thread);
    LeaveCriticalZone();
}

size_t sched::CScheduler::GetThreadCount() const
{
    return m_ThreadCount;
}

void sched::CScheduler::RemoveThread(CThread *thread)
{
    thread_list* tmp = m_Threads;
    thread_list* oldTmp;
    while(tmp->Thread != thread)
    {
        oldTmp = tmp;
        tmp = tmp->Next;
    }

    oldTmp->Next = tmp->Next;
}

sched::CThread*& sched::CScheduler::GetCurrentThread()
{
    auto thiz = GetInstance();
    return thiz.m_Threads->GetByIndex(GetCurrentThreadId());
}

int sched::CScheduler::GetCurrentThreadId()
{
    auto thiz = GetInstance();
    return thiz.m_Index;
}

sched::CThread*& sched::CScheduler::GetThread(thread_t id)
{
    auto thiz = GetInstance();
    return thiz.m_Threads->GetByIndex(id);
}
