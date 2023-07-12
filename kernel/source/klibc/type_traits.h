#pragma once

namespace traits
{

template<class _Ty> 
struct remove_reference
{
    using type = _Ty;
};

template<class _Ty>
struct remove_reference<_Ty&>
{
    using type = _Ty;
};

}

template<class _Ty>
constexpr typename traits::remove_reference<_Ty>::type&& \
Move(_Ty&& value) noexcept
{
    return static_cast<typename traits::remove_reference<_Ty>::type&&>(value);
}

template<class _Ty>
void Swap(_Ty& arg1, _Ty& arg2)
{
    _Ty temp = Move(arg1);
    arg1 = Move(arg2);
    arg2 = temp;
}
