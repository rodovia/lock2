#pragma once

#include "arch/i386/cpu/idt.h"

namespace sched
{

using ::full_register_state;

using thread_start_routine = void(*)(void*);

enum thread_creation_flags
{
    kThreadCreateSuspended = 0x001,
    kThreadKernelMode = 0x010
};

enum thread_suspend_reason
{
    kThreadSuspendReasonNotSuspended,
    kThreadSuspendReasonKilled,
    kThreadSuspendReasonReturned, /* thread_start_routine returned. */
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
            thread_creation_flags flags
        );

    thread_t GetId();
    void Start();
    full_register_state* GetState() const;
    void SaveState(full_register_state state);
    void SetSuspended(bool value, 
                    thread_suspend_reason reason = kThreadSuspendReasonKilled
                );
private:
    bool m_Suspended;
    thread_suspend_reason m_SuspendReason;
    full_register_state* m_RegState;
};

}
