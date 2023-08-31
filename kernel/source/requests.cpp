#include <stddef.h>
#include "limine.h"
#include "requests.h"
#include "terminal.h"

static volatile struct limine_framebuffer_request 
framebuffer = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_module_request
mdls = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 1
};

static volatile struct limine_memmap_request
mp = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request
hhdm = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request
ke = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_rsdp_request
rsdp = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static volatile struct limine_efi_system_table_request
efi = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0,
};

static volatile struct limine_kernel_file_request
kf = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0,  
};

[[gnu::section(".limine_reqs")]] [[gnu::used]]
static volatile void*
limineReqs[] = \
{
    reinterpret_cast<volatile void*>(&framebuffer),
    reinterpret_cast<volatile void*>(&mdls),
    reinterpret_cast<volatile void*>(&mp),
    reinterpret_cast<volatile void*>(&hhdm),
    reinterpret_cast<volatile void*>(&ke),
    reinterpret_cast<volatile void*>(&rsdp),
    reinterpret_cast<volatile void*>(&efi),
    reinterpret_cast<volatile void*>(&kf),
    nullptr
};

struct limine_framebuffer_response* 
rqs::GetFramebuffer()
{
    return framebuffer.response;
}

struct limine_module_response* 
rqs::GetModules()
{
    return mdls.response;
}

struct limine_memmap_response*
rqs::GetMemoryMap()
{
    return mp.response;
}

struct limine_hhdm_response*
rqs::GetHhdm()
{
    return hhdm.response;
}

struct limine_kernel_address_response*
rqs::GetAddresses()
{
    return ke.response;
}

struct limine_rsdp_response*
rqs::GetRsdp()
{
    return rsdp.response;
}

bool
rqs::IsEfi()
{
    return efi.response != nullptr;
}

struct limine_kernel_file_response*
rqs::GetKernelFile()
{
    return kf.response;
}
