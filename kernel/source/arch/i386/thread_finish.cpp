#include "scheduler/scheduler.h"

/* 'Called' by a thread when it returns. */
extern "C"
void _ThreadFinish()
{
    auto thread = sched::CScheduler::GetCurrentThread();
    auto sch = sched::CScheduler::GetInstance();
    sch.RemoveThread(thread);
    while (true)
    {
        asm ("hlt");
    }
}
