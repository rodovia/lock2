#include "alloc/physical.h"
#include "scheduler/scheduler.h"

/* 'Called' by a thread when it returns. */
extern "C"
void _ThreadFinish()
{
    auto thread = sched::CScheduler::GetCurrentThread();
    auto& sch = sched::CScheduler::GetInstance();
    sch.RemoveThread(thread);
    sch.YieldThreadTime();

    while (true)
    {
        asm ("hlt");
    }
}
