#pragma once

#include <stdint.h>
#include <stddef.h>

#include "dhelp_driver.h"

enum driver_role
{
    kDHelpDriverRoleUninit, /*< Driver did not call SetRole or 
                            its init function was not called yet. */
    kDHelpDriverRoleDisk,
};

enum driver_result
{
    kDHelpResultOk,
    kDHelpResultInvalidArgument
};


class IDHelpTerminal
{
public:
    virtual ~IDHelpTerminal() __system {};

    virtual void WriteString(const char* string, size_t length) __system __pure;
    virtual void WriteFormat(const char* string, ...) __system __pure;
};

class IDHelpMutex
{
public:
    virtual ~IDHelpMutex() __system {}

    virtual void Lock() __system __pure;
    /* It's literally the kernel's name can you believe it! */
    virtual void Lock2(int timeout) __system __pure;
    virtual void Release() __system __pure;
};

class IDHelpSemaphore
{
public:

};

class IDHelpThreadScheduler
{
public:
    virtual ~IDHelpThreadScheduler() __system {}

    virtual void HaltExecution(int ms) __system __pure;
    virtual void CreateMutex(IDHelpMutex** mutx) __system __pure;
    // virtual void CreateSemaphore(int limit, IDHelpSemaphore** sem) __system __pure;
    /* TODO: add CreateThread etc */
};

class IDHelpPciDevice
{
public:
    virtual ~IDHelpPciDevice() __system {};

    virtual bool IsValid() const __system __pure;
    virtual uint16_t GetVendor() const __system __pure;
    virtual uint8_t GetDevice() const __system __pure;
    virtual uint16_t GetClassSubClass() const __system __pure;
    virtual uint32_t ReadConfigurationSpace(uint8_t offset) const __system __pure;
    virtual void WriteConfigurationSpace(uint8_t offset, uint32_t nvalue) __system __pure;
};

class IDHelpPci
{
public:
    virtual ~IDHelpPci() __system {};

    virtual void EnumerateDevices(IDHelpPciDevice** outDevices, 
                                    size_t* outLength) __system __pure;
    virtual void FindDeviceExact(uint8_t bus, uint8_t device, 
                                 IDHelpPciDevice** outDevice) __system __pure;
    virtual void FindDeviceByClass(uint8_t klass, uint8_t subclass, 
                                   IDHelpPciDevice** outDevice) __system __pure;
};

class IDHelpDriverManager
{
public:
    virtual ~IDHelpDriverManager() __system {};

    virtual int GetPci(IDHelpPci** pci) __system __pure;
    virtual int GetTerminal(IDHelpTerminal** term) __system __pure;
    virtual int GetInterruptController(class IDHelpInterruptController** ic) __system __pure;
    virtual int GetAllocator(class IDHelpMemoryAllocator** al) __system __pure;
    virtual int GetThreadManager(class IDHelpThreadScheduler** thr) __system __pure;
    virtual void FreeInterface(void* ref) __system __pure;

    virtual void SetRole(driver_role role) noexcept __system __pure;
    virtual int GetRole() noexcept __system __pure;
    virtual void SetInterface(void* interf) noexcept __system __pure;
};

class IDHelpInterruptController
{
public:
    virtual ~IDHelpInterruptController() __system { };
    virtual int GenerateVector() __system __pure;
    virtual void RemoveVector(int vector) __system __pure;
    virtual void HandleInterrupt(int vector, driver_interrupt_handler handler, void* context) __system __pure;
    virtual void AssociateVector(int pvector, int vvector) __system __pure;
};

class IDHelpMemoryAllocator
{
public:
    virtual ~IDHelpMemoryAllocator() __system { };
    virtual void* Allocate(size_t bytes) __system __pure;
    virtual void Free(void* block) __system __pure;
    virtual void* AlignedAlloc(size_t bytes, uint16_t alignment) __system __pure;
    virtual void AlignedFree(void* block, size_t bytes) __system __pure;
};
