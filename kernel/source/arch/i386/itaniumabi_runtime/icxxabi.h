#pragma once

#define _ATEXIT_MAX_FUNCS 128

typedef unsigned uarch_t;
typedef void (*destructor_func_t)(void*);

struct atexit_func_entry_t
{
    destructor_func_t destructor_func;
    void* obj_ptr;
    void* dso_handle;
};

#ifdef __cplusplus
extern "C"
{
#endif

int __cxa_atexit(destructor_func_t destructor, void* obj_ptr, void* dso_handle);
void __cxa_finalize(void* f);

#ifdef __cplusplus
}
#endif
