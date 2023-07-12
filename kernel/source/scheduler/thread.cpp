#include "thread.h"
#include "alloc/physical.h"
#include "arch/i386/cpu/idt.h"
#include "scheduler/scheduler.h"
#include "terminal.h"

static void _ThreadWrapper(sched::CThread* thiz, 
                            void* routine, void* data)
{
    sched::CScheduler& scheduler = sched::CScheduler::GetInstance();
    auto threadMain = reinterpret_cast<sched::thread_start_routine>(routine);
    threadMain(data);

    thiz->SetSuspended(true, sched::kThreadSuspendReasonReturned);
    scheduler.RemoveThread(thiz);
    while(true)
    {
        asm volatile("hlt");
    }
}

sched::CThread::CThread(sched::thread_start_routine routine,
                        void* data,
                        sched::thread_creation_flags flags)
    : m_Suspended(true),
      m_SuspendReason(kThreadSuspendReasonWaiting)
{
    m_RegState = new full_register_state;
    m_RegState->Pointers.Cs = (flags & kThreadKernelMode) ? 3 * 8 
                                                         : 6 * 8;
    m_RegState->Pointers.Rip = (uword)_ThreadWrapper;
    m_RegState->Pointers.Ss = 4 * 8;
    m_RegState->Pointers.Rsp = (uword)pm::AlignedAlloc(4096, 8);
    m_RegState->OtherRegisters.Rdi = (uword)this;
    m_RegState->OtherRegisters.Rsi = (uword)routine;
    m_RegState->OtherRegisters.Rdx = (uword)data;

    if (!(flags & kThreadCreateSuspended))
    {
        this->Start();
    }
}

void sched::CThread::SetSuspended(bool value, thread_suspend_reason reason)
{
    m_Suspended = value;
    m_SuspendReason = reason;
}

void sched::CThread::Start()
{
    m_Suspended = false;
    m_SuspendReason = kThreadSuspendReasonNotSuspended;
}

void sched::CThread::SaveState(full_register_state state)
{
    *m_RegState = state;
}

full_register_state* sched::CThread::GetState() const
{
    return m_RegState;
}
