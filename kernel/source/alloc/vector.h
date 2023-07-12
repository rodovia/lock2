#pragma once

#include "alloc/physical.h"
#include "arch/i386/paging/paging.h"
#include "terminal.h"
#include <stddef.h>

template<class _Ty>
class CVector
{
public:
    CVector(int initialCapacity)
        : m_Buffer(nullptr),
        m_Length(0),
        m_Capacity(initialCapacity)
    {
        if (initialCapacity > 0)
        {
            m_Buffer = new _Ty[initialCapacity];
        }
    }

    CVector()
        : m_Length(0),
        m_Capacity(3)
    {
        m_Buffer = new _Ty[m_Capacity];
    }

    ~CVector()
    {
        if (m_Buffer != nullptr)
        {
            delete[] m_Buffer;
        }
    }

    void PushBack(_Ty& item)
    {
        if (m_Length >= m_Capacity)
        {
            this->Reserve(m_Capacity + 3);
        }

        m_Buffer[m_Length] = item;
        m_Length++;
    }

    _Ty& Get(size_t idx)
    {
        return m_Buffer[idx];
    }
    
    const _Ty& Get(size_t idx) const
    {
        return m_Buffer[idx];
    }

    _Ty& operator[](size_t idx)
    {
        return this->Get(idx);
    }

    const _Ty& operator[](size_t idx) const
    {
        return this->Get(idx);
    }

    size_t Length() const
    {
        return m_Length;
    }
    
    void Reserve(int capacity)
    {
        if ((size_t)capacity <= m_Capacity)
        {
            return;
        }

        m_Capacity += capacity;
        _Ty* buffer = new _Ty[m_Capacity];

        memcpy(buffer, m_Buffer, m_Length);
        delete[] m_Buffer;
        m_Buffer = buffer;
    }
    
    _Ty* m_Buffer;
    size_t m_Length;
    size_t m_Capacity;
};
