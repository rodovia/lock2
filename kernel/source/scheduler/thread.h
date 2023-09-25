#pragma once

#include "arch/i386/cpu/idt.h"
#include "arch/i386/timer/time_units.h"

namespace sched
{

using ::full_register_state;

using thread_start_routine = void(*)(void*);
using async_timer_routine = void(*)(void*);

enum thread_creation_flags
{
    kThreadCreateSuspended = 0x001,
    kThreadKernelMode = 0x010
};

struct thread_async_timer_data
{
    time::millisec_t Ticks;
    async_timer_routine Routine;
    void* Data;
};

void Sleep(time::millisec_t ticks);
void AsyncTimer(time::millisec_t ticks, 
                async_timer_routine routine,
                void* data);

enum thread_suspend_reason : unsigned int
{
    kThreadSuspendReasonNotSuspended,
    kThreadSuspendReasonKilled,
    kThreadSuspendReasonReturned, /* thread_start_routine returned. */
    kThreadSuspendReasonSleeping, /* thread called Sleep. */
    kThreadSuspendReasonWaiting /* Thread created with kThreadCreateSuspended and
                                    is still waiting to startup or waiting for
                                    resource */
};

using thread_t = int;

class CThread
{
    friend class CScheduler;
public:
    CThread() = default;
    CThread(thread_start_routine routine,
            void* data,
            thread_creation_flags flags);
    CThread(CThread&&) = delete;
    CThread(const CThread&) = delete;
    ~CThread();

    void Start();
    full_register_state* GetState() const;
    void SaveState(full_register_state state);
    void SetSuspended(bool value, 
                    thread_suspend_reason reason = kThreadSuspendReasonKilled);
    int GetId() const;
private:
    int m_Id;
    bool m_Suspended;
    thread_suspend_reason m_SuspendReason;
    full_register_state* m_RegState;
    void* m_StackStart;
};

}
