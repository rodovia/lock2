#include "icxxabi.h"
#include "terminal.h"

extern "C"
{

atexit_func_entry_t __atexit_funcs[_ATEXIT_MAX_FUNCS];
uarch_t __atexit_func_count = 0;

void* __dso_handle = nullptr;

}

int __cxa_atexit(destructor_func_t destructor, void *obj_ptr, void *dso_handle)
{
    if (__atexit_func_count >= _ATEXIT_MAX_FUNCS)
    {
        return -1;
    }

    __atexit_funcs[__atexit_func_count].destructor_func = destructor;
    __atexit_funcs[__atexit_func_count].obj_ptr = obj_ptr;
    __atexit_funcs[__atexit_func_count].dso_handle = dso_handle;
    __atexit_func_count++;
    return 0;
}

void __cxa_finalize(void *f)
{
    uarch_t i = __atexit_func_count;
    if (f == nullptr)
    {
        while (i--)
        {
            if (__atexit_funcs[i].destructor_func == nullptr)
            {
                /* So we won't execute the IVT... */
                continue;
            }

            (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
        }

        return;
    }

    if (__atexit_funcs[i].destructor_func == f)
    {
        (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
        __atexit_funcs[i].destructor_func = nullptr;
    }
}

extern "C" void __cxa_pure_virtual()
{
    Error("__cxa_pure_virtual was called.\n");
}
