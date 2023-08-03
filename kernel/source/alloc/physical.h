#pragma once

#include <stddef.h>
#include <stdint.h>

#define BochsDebugBreak asm volatile("xchg %bx, %bx")
#define PaAdd(B, s) reinterpret_cast<void*>(reinterpret_cast<uint64_t>(B) + s)
#define ZeroMemory(T, s) memset(T, 0, s)

struct mem_global;

extern "C" void* memset(void* to, int value, size_t size);
extern "C" void* memcpy(void* to, const void* from, size_t size);
extern "C" void* memmove(void* to, const void* from, size_t size);

namespace pm
{

mem_global* Create(void* begin, size_t length);
void* GetGlobal();
void* GetBegin();
void* GetEnd();

void* Alloc(size_t bytes);
void* AlignedAlloc(size_t bytes, uint16_t aligned);
void Free(void* block);
void AlignedFree(size_t size, void* block);
bool WasAlloqued(void* ptr);

}

