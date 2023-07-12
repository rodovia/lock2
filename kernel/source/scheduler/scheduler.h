#pragma once

#include "scheduler/thread.h"
#include <vector>

namespace sched
{

struct thread_list
{
    thread_list()
        : Thread(nullptr),
          Next(nullptr)
    {
    }

    thread_list(CThread* thread)
        : Thread(thread),
          Next(nullptr)
    {}

    CThread*& GetByIndex(size_t offset) noexcept
    {
        size_t index = 0;
        thread_list* tmp = this;
        while (tmp->Next != nullptr)
        {
            if (index == offset)
            {
                break;
            }

            index++;
            tmp = tmp->Next;
        }

        return tmp->Thread;
    }

    CThread* Thread;
    thread_list* Next;
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
    void AddThread(CThread* thread);
    void RemoveThread(CThread* thread);
    void Enable();
    size_t GetThreadCount() const;
private:
    void ThinkDeep(full_register_state regState);

    bool m_Enabled;
    thread_list* m_Threads;
    uint32_t m_ThreadCount;
    uint32_t m_Quantum;
    uint32_t m_Index;
};

}
