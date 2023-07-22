#include "thread.h"
#include "alloc/physical.h"
#include "arch/i386/cpu/idt.h"
#include "arch/i386/timer/time_units.h"
#include "scheduler/scheduler.h"
#include "terminal.h"

static void AsyncTimerThread(void* data)
{
    auto tmr = reinterpret_cast<sched::thread_async_timer_data*>(data);
    if (tmr->Routine == nullptr)
    {
        Warn("Async timer callback is nullptr!\n");
        return;
    }

    sched::Sleep(tmr->Ticks);
    tmr->Routine(tmr->Data);
}

void sched::Sleep(time::millisec_t ticks)
{
    auto t = CScheduler::GetCurrentThread();
    auto sche = CScheduler::GetInstance();
    t->SetSuspended(true, kThreadSuspendReasonWaiting);
    sche.AddSuspendedThread(t, ticks);
}

void sched::AsyncTimer(time::millisec_t ticks, 
                        async_timer_routine routine,
                        void* data)
{
    auto d = new thread_async_timer_data();
    d->Data = data;
    d->Routine = routine;
    d->Ticks = ticks;

    CThread* thread = new CThread(AsyncTimerThread, d, kThreadKernelMode);
    sched::CScheduler::GetInstance().AddThread(thread);
}

sched::CThread::CThread(sched::thread_start_routine routine,
                        void* data,
                        sched::thread_creation_flags flags)
    : m_Suspended(true),
      m_SuspendReason(kThreadSuspendReasonWaiting)
{
    m_StackStart = pm::Alloc(4096);
    m_RegState = (full_register_state*)pm::Alloc(sizeof(full_register_state));
    memset(m_RegState, 0, sizeof(full_register_state));

    m_RegState->Pointers.Cs = (flags & kThreadKernelMode) ? 3 * 8
                                                         : 6 * 8;
    m_RegState->Pointers.Rip = (uword)routine;
    m_RegState->Pointers.Ss = 4 * 8;
    m_RegState->Pointers.Rsp = (uword)m_StackStart + 4096;
    m_RegState->OtherRegisters.Rdi = (uword)data;

    if (!(flags & kThreadCreateSuspended))
    {
        this->Start();
    }
}

sched::CThread::~CThread()
{
    pm::Free(m_RegState);
    pm::Free(m_StackStart);
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

