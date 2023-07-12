/* ACPICA OS Layer impl */

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

#pragma GCC diagnostic ignored "-Wunused-parameter"

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
        sem->Wait();
    }
    return AE_TIME;
}