/* ACPICA OS Layer impl */

/* Yes, this file breaks all of Lock2's coding conventions...
  ...Just to adhere to ACPICA ones. 
  Try to fix it, if you are not lazy. =) */

#include "arch/i386/port.h"
#include "arch/i386/spinlock.h"
#include "arch/i386/timer/tsc.h"
#include "scheduler/scheduler.h"
#include "scheduler/semaphore.h"
#include "scheduler/thread.h"
#include <cstdarg>

#include "terminal.h"
#include "alloc/physical.h"
#include "arch/i386/paging/paging.h"

extern "C"
{
#include "acpica/acpi.h"
#include "acpi/acpica/actypes.h"
#include "acpica/acpiosxf.h"
}

[[gnu::always_inline]]
static inline void cli()
{
    asm volatile("cli");
}

[[gnu::always_inline]]
static inline void sti()
{
    asm volatile("sti");
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

extern "C"
{

ACPI_STATUS 
AcpiOsInitialize()
{
    return AE_OK;
}

ACPI_STATUS 
AcpiOsTerminate()
{
    return AE_OK;
}

ACPI_PHYSICAL_ADDRESS 
AcpiOsGetRootPointer()
{
    ACPI_PHYSICAL_ADDRESS root = 0;
    AcpiFindRootPointer(&root);
    return root;
}

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *InitVal, ACPI_STRING *NewVal)
{
    (*NewVal) = nullptr;
    return AE_OK;
}

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable)
{
    (*NewTable) = nullptr;
    return AE_OK;
}

void*
AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length)
{
    virtm::MapPages(nullptr, Where, Where, PT_FLAG_WRITE);
    return (void*)Where;
}

void 
AcpiOsUnmapMemory(void* Where, ACPI_SIZE)
{
    virtm::UnmapPages(nullptr, (vaddr_t)Where);
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void *LogicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress)
{
    (*PhysicalAddress) = (ACPI_PHYSICAL_ADDRESS)virtm::WalkPageTable(nullptr, (vaddr_t)LogicalAddress);
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size)
{
    return pm::Alloc(Size);
}

void AcpiOsFree(void* Memory)
{
    return pm::Free(Memory);
}

void* AcpiOsAllocateZeroed(ACPI_SIZE Size)
{
    void* p = pm::Alloc(Size);
    memset(p, 0, Size);
    return p;
}

void
AcpiOsVprintf(const char* Format, va_list Args)
{
    CTerminal::WriteFormatted(Format, Args);
}

void
AcpiOsPrintf(const char* Format, ...)
{
    va_list v;
    va_start(v, Format);
    CTerminal::WriteFormatted(Format, v);
    va_end(v);
}

ACPI_THREAD_ID
AcpiOsGetThreadId()
{
    return sched::CScheduler::GetCurrentThreadId();
}

ACPI_STATUS
AcpiOsExecute(ACPI_EXECUTE_TYPE, ACPI_OSD_EXEC_CALLBACK Function, void *Context)
{
    auto thr = new sched::CThread(Function, Context, sched::kThreadKernelMode);
    auto sch = sched::CScheduler::GetInstance();
    sch.AddThread(thr);
    return AE_OK;
}

ACPI_STATUS 
AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle)
{
    auto sem = new sched::semaphore(InitialUnits);
    (*OutHandle) = sem;
    return AE_OK;
}

ACPI_STATUS
AcpiOsDeleteSemaphore(void *Handle)
{
    delete reinterpret_cast<sched::semaphore*>(Handle);
    return AE_OK;
}

ACPI_STATUS 
AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
    sched::semaphore* sem = reinterpret_cast<sched::semaphore*>(Handle);
    if (Timeout == 0)
    {
        if (!sem->IsFree())
        {
            return AE_TIME;
        }

        sem->Wait();
        return AE_OK;
    }

    if (Timeout == (uint16_t)-1)
    {
        sem->Wait();
        return AE_OK;
    }

    int result = sem->Wait(Timeout);
    return result == sched::kSemaphoreErrorOk ? AE_OK : AE_TIME;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
    auto sem = reinterpret_cast<sched::semaphore*>(Handle);
    for (uint32_t i = 0; i < Units; i++)
    {
        sem->Notify();
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* spl)
{
    if (spl == nullptr)
    {
        return AE_BAD_PARAMETER;
    }

    (*spl) = new spinlock;
    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK spl)
{
    delete (spinlock*)spl;
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(void* Handle)
{
    SlLockAcquire((spinlock*)Handle);
    cli();
    return 0;
}

void AcpiOsReleaseLock(void* Handle, ACPI_CPU_FLAGS)
{
    SlLockRelease((spinlock*)Handle);
    sti();
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info)
{
    switch (Function)
    {
    case ACPI_SIGNAL_FATAL:
    {
        auto fat = reinterpret_cast<ACPI_SIGNAL_FATAL_INFO*>(Info);
        Error("FATAL opcode inside ACPI! Type %i Code 0x%p Argument 0x%p\n",
                fat->Type, fat->Code, fat->Argument);
        break;
    }
    case ACPI_SIGNAL_BREAKPOINT:
    {
        Info("ACPI: Breakpoint hit! But Lock2 does not support AML debugging.\n");
        break;
    }
    }

    return AE_OK;
}

UINT64 AcpiOsGetTimer()
{
    return TscRead();
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address,
                            UINT32* Value,
                            UINT32 Width)
{
    if (Width == 8)
    {
        port8 p = Address;
        (*Value) = (uint8_t)*p;
        return AE_OK;
    }
    else if (Width == 16)
    {
        port16 p = Address;
        (*Value) = (uint16_t)*p;
        return AE_OK;
    }
    else if (Width == 32)
    {
        port32 p = Address;
        (*Value) = (uint32_t)*p;
        return AE_OK;
    }

    return AE_OK; /* Unreachable */
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address,
                            UINT32 Value,
                            UINT32 Width)
{
    if (Width == 8)
    {
        port8 p = Address;
        (*p) = Value;
        return AE_OK;
    }
    else if (Width == 16)
    {
        port16 p = Address;
        (*p) = Value;
        return AE_OK;
    }
    else if (Width == 32)
    {
        port32 p = Address;
        (*p) = Value;
        return AE_OK;
    }

    return AE_OK; /* Unreachable */
}



}