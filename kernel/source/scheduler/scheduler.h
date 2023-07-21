#pragma once

#include "arch/i386/timer/time_units.h"
#include "scheduler/thread.h"
#include <vector>

namespace sched
{

template<class _Ty>
struct linked_list
{
    using ValueType = _Ty;
    linked_list()
        : Item(nullptr),
          Next(nullptr)
    {
    }

    linked_list(_Ty* thread)
        : Item(thread),
          Next(nullptr)
    {}

    constexpr void Append(_Ty* data) noexcept
    {
        linked_list<_Ty>* tmp = this;
        while (tmp->Next != nullptr)
        {
            tmp = tmp->Next;
        }
        tmp->Next = new linked_list<_Ty>(data);
    }

    constexpr _Ty*& GetByIndex(size_t offset) noexcept
    {
        size_t index = 0;
        linked_list<_Ty>* tmp = this;
        while (tmp->Next != nullptr)
        {
            if (index == offset)
            {
                break;
            }

            index++;
            tmp = tmp->Next;
        }

        return tmp->Item;
    }

    _Ty* Item;
    linked_list<_Ty>* Next;
};

using thread_list = linked_list<CThread>;

struct thread_sleep_info
{
    CThread* Thread;
    time::millisec_t RemainingTicks;
};

class CScheduler
{
public:
    CScheduler();
    static CScheduler& GetInstance()
    {
        static CScheduler s;
        return s;
    }

    static CThread*& GetCurrentThread();
    static int GetCurrentThreadId();
    static void Think(full_register_state regState);
    static CThread*& GetThread(thread_t id);
    void AddSuspendedThread(CThread* thread, time::millisec_t ticks);
    void RemoveSuspendedThread(thread_sleep_info* info);
    void AddThread(CThread* thread);
    void RemoveThread(CThread* thread);
    void Enable();
    size_t GetThreadCount() const;

private:
    void ThinkDeep(full_register_state regState);
    void ThinkSuspendedThreads();

    bool m_Enabled;
    linked_list<thread_sleep_info>* m_SuspendedThreads;
    thread_list* m_Threads;
    uint32_t m_ThreadCount;
    uint32_t m_Quantum;
    uint32_t m_Index;
};

}
