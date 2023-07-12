#pragma once

#include "limine.h"

namespace rqs 
{

struct limine_framebuffer_response* GetFramebuffer();
struct limine_module_response* GetModules();
struct limine_memmap_response* GetMemoryMap();
struct limine_hhdm_response* GetHhdm();
struct limine_kernel_address_response* GetAddresses();
struct limine_rsdp_response* GetRsdp();
bool IsEfi();
struct limine_kernel_file_response* GetKernelFile();

}
