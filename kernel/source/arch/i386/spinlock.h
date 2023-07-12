#pragma once

#include <stdint.h>

struct spinlock
{
    /* For some reason, the BTS instruction requires the 
        destination operand to be r/w as a qword, even when 
        (in our case) we only use the FIRST BIT of the FIRST BYTE.
    I hate this. */
    uint64_t flag;
};

extern "C"
{

void SlLockAcquire(spinlock* sp);
void SlLockRelease(spinlock* sp);

}