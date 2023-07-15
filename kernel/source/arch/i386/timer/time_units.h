#pragma once

#include <stdint.h>

/* Time measurement units (nanosecond, millisecond, second etc) */

namespace time
{

using nanosec_t = uint64_t;
using millisec_t = uint64_t;
using second_t = uint64_t;

}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuser-defined-literals" 

constexpr inline time::nanosec_t operator ""ms(unsigned long long ms)
{
    return ms * 1000;
}

constexpr inline time::nanosec_t operator ""s(unsigned long long s)
{
    return s * 1000000000;
}

#pragma GCC diagnostic pop


