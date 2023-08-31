#pragma once

#include "alloc/physical.h"
#include <type_traits>
#include <stdint.h>

#ifndef __system
#define __system __attribute__((sysv_abi))
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++20-extensions" /* Usage of asm keyword 
                                                    inside constexpr is C++20 */

namespace detail
{

/* Took from <https://stackoverflow.com/questions/64945691> */
inline void ins(uint16_t port, uint8_t* dest, uint32_t count) 
{
    asm volatile("rep insb"
        : "+D" (dest), "+c" (count), "=m" (*dest)
        : "d" (port)
        : "memory");
}

/* Clang does not support %z */
inline void ins(uint16_t port, uint16_t* dest, uint32_t count) 
{
    asm volatile("rep insw"
        : "+D" (dest), "+c" (count), "=m" (*dest)
        : "d" (port)
        : "memory");
}

inline void ins(uint16_t port, uint32_t* dest, uint32_t count) 
{
    asm volatile("rep insl"
        : "+D" (dest), "+c" (count), "=m" (*dest)
        : "d" (port)
        : "memory");
}

}

template<class ReadSize>
class CPortRef
{

    template<class _> friend class CPort;
public:
    constexpr ReadSize Read()
    {
        ReadSize res;
        if constexpr (std::is_same_v<ReadSize, uint8_t>)
        {
            asm volatile("inb %1, %0" :  "=a"(res) : "d"(m_Port));
        }
        else if constexpr (std::is_same_v<ReadSize, uint16_t>)
        {
            asm volatile("inw %1, %0" : "=a"(res) : "d"(m_Port));
        }
        else if constexpr (std::is_same_v<ReadSize, uint32_t>)
        {
            asm volatile("inl %1, %0" : "=a"(res) : "d"(m_Port));
        }

        return res;
    }

    constexpr void Write(ReadSize res)
    {
        if constexpr (std::is_same_v<ReadSize, uint8_t>)
        {
            asm volatile ("outb %0, %1" : : "a"(res), "Nd"(m_Port) : "memory");
        }
        else if constexpr (std::is_same_v<ReadSize, uint16_t>)
        {
            asm volatile ("outw %0, %1" : : "a"(res), "Nd"(m_Port) : "memory");
        }
        else if constexpr(std::is_same_v<ReadSize, uint32_t>)
        {
           asm volatile ("outl %0, %1" : : "a"(res), "Nd"(m_Port) : "memory");
        }
    }

    constexpr void ReadRepeated(ReadSize* buffer, size_t size) __system
    {
        detail::ins(m_Port, buffer, size);
    }

    constexpr void operator=(ReadSize value)
    {
        this->Write(value);
    }

    constexpr operator ReadSize()
    {
        return this->Read();
    }

private:
    CPortRef(uint16_t port)
        : m_Port(port)
    {}

    uint16_t m_Port;
};

template<class ReadSize>
class CPort
{
public:
    static_assert(std::is_integral<ReadSize>::value && 
                !std::is_same_v<ReadSize, uint64_t>);
    using OperationSize = ReadSize;

    CPort(uint16_t port)
        : m_Port(port)
    {};

    constexpr CPortRef<ReadSize> operator*()
    {
        return CPortRef<ReadSize>{m_Port};
    }

private:
    uint16_t m_Port;
};

using port8 = CPort<unsigned char>;
using port16 = CPort<unsigned short>;
using port32 = CPort<unsigned int>;

template<class ReadSize>
constexpr CPort<ReadSize> GetRegister(uint16_t port, uint16_t reg)
{
    return port + reg;
}

#pragma GCC diagnostic pop
