/******************************************************************************
 *
 * Name: aclock2.h - OS specific defines, etc. for Lock2
 * (Based off aclinux.h)
 *
 *****************************************************************************/

#pragma once

#include <stdint.h>

/* File made for Lock2. Not under ACPICA's license */

#ifndef __LOCK2_H__
#define __LOCK2_H__
/* Common (in-kernel/user-space) ACPICA configuration */

// #define ACPI_USE_SYSTEM_CLIBRARY
#define ACPI_USE_DO_WHILE_0
// #define ACPI_IGNORE_PACKAGE_RESOLUTION_ERRORS

#ifdef __x86_64__
#define ACPI_FLUSH_CPU_CACHE() asm volatile ("wbinvd")
#endif

#define ACPI_USE_GPE_POLLING

/* Kernel specific ACPICA configuration */

#ifdef CONFIG_ACPI_REDUCED_HARDWARE_ONLY
#define ACPI_REDUCED_HARDWARE 1
#endif

#ifdef CONFIG_ACPI_DEBUGGER
#define ACPI_DEBUGGER
#endif

#ifdef CONFIG_ACPI_DEBUG
#define ACPI_MUTEX_DEBUG
#endif

#ifndef offsetof
#define offsetof(D, f) __builtin_offsetof(D, f)
#endif

/* Host-dependent types and defines for in-kernel ACPICA */

/* I do not expect to port Lock2 to non-64-bit archs */
#define ACPI_MACHINE_WIDTH          64
#define ACPI_USE_NATIVE_MATH64
#define ACPI_CACHE_T                ACPI_MEMORY_LIST
#define ACPI_USE_LOCAL_CACHE        1
#define ACPI_CPU_FLAGS              unsigned long
#define ACPI_UINTPTR_T              uintptr_t
#define ACPI_TO_INTEGER(p)          ((uintptr_t)(p))
#define ACPI_OFFSET(d, f)           offsetof(d, f)
#define USE_NATIVE_ALLOCATE_ZEROED

/*
 * OSL interfaces used by debugger/disassembler
 */
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsReadable
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsWritable
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsInitializeDebugger
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsTerminateDebugger

/*
 * OSL interfaces used by utilities
 */
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsRedirectOutput
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByName
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByIndex
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByAddress
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsOpenDirectory
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetNextFilename
#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsCloseDirectory

#define ACPI_MSG_ERROR          "ACPI Error: "
#define ACPI_MSG_EXCEPTION      "ACPI Exception: "
#define ACPI_MSG_WARNING        "ACPI Warning: "
#define ACPI_MSG_INFO           "ACPI: "

#define ACPI_MSG_BIOS_ERROR     "ACPI BIOS Error (bug): "
#define ACPI_MSG_BIOS_WARNING   "ACPI BIOS Warning (bug): "

#endif
