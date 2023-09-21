#pragma once

/* By default, Clang (when building the drivers
    will think these methods are ms_abi. We don't want that. */
#define __system __attribute__((sysv_abi))
#define __pure = 0

typedef void(*driver_interrupt_handler)(void* context) __system;
