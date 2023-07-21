#include "scheduler.h"
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

    m_Index++;
    if (m_Index > m_ThreadCount)
    {
        m_Index = 0;
    }
    
    acpi::EndOfInterrupt();
    CThread*& current = m_Threads->GetByIndex(m_Index);
    if (current == nullptr)
    {
        return;
    }

    m_Quantum = 10;
    SchSwitchTaskKernel(current->m_RegState);
}

void sched::CScheduler::AddThread(CThread* thread)
{
    EnterCriticalZone();
    if (m_Threads == nullptr)
    {
        m_Threads = new thread_list;
    }

    m_Threads->Append(thread);
    m_ThreadCount++;
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
    while(tmp->Item != thread)
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

void sched::CScheduler::AddSuspendedThread(CThread *thread, time::millisec_t ticks)
{
    EnterCriticalZone();
    if (m_SuspendedThreads == nullptr)
    {
        m_SuspendedThreads = new linked_list<thread_sleep_info>();
    }

    auto t = new thread_sleep_info{thread, ticks};
    m_SuspendedThreads->Append(t);
    LeaveCriticalZone();
}

void sched::CScheduler::RemoveSuspendedThread(thread_sleep_info* i)
{
    linked_list<thread_sleep_info>* tmp = m_SuspendedThreads,
                                  *  oldtmp;

    while (tmp->Next != nullptr)
    {
        if (tmp->Item == i)
        {
            oldtmp->Next = tmp->Next;
            return;            
        }

        oldtmp = tmp;
        tmp = tmp->Next;
    }
}

void sched::CScheduler::ThinkSuspendedThreads()
{
    auto head = m_SuspendedThreads;
    if (head == nullptr)
    {
        return;
    }

    while (head->Next != nullptr)
    {
        auto item = head->Item;
        item->RemainingTicks--;
        if (head->Item->RemainingTicks <= 0)
        {
            this->RemoveSuspendedThread(item);
            item->Thread->SetSuspended(false, kThreadSuspendReasonNotSuspended);
        }

        head = head->Next;
    }
}
