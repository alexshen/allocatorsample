//
//  memory_utils.h
//  memoryallocator
//
//  Created by ashen on 2019/8/20.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <cassert>

namespace memory
{
    
namespace detail
{

template<typename T>
constexpr std::size_t alignment()
{
    return alignof(T);
}
    
template<>
constexpr std::size_t alignment<void>()
{
    return 1;
}
    
} // namespace detail
    
constexpr bool isPowerOfTwo(std::size_t n)
{
    return n & ~(n - 1);
}
    
constexpr bool isValidAlignment(std::size_t alignment)
{
    return alignment && isPowerOfTwo(alignment);
}
    
inline std::size_t roundUp(std::size_t size, std::size_t n)
{
    assert(n);
    return (size + n - 1) / n * n;
}
    
inline std::size_t roundDownPowerOfTwo(std::size_t size, size_t align)
{
    assert(isValidAlignment(align));
    return size & ~(align - 1);
}
    
inline std::size_t roundUpPowerOfTwo(std::size_t size, std::size_t align)
{
    assert(isValidAlignment(align));
    return (size + align - 1) & ~(align - 1);
}
    
template<typename T>
inline T* align(T* p, std::size_t align, std::size_t offset = 0)
{
    assert(isValidAlignment(align));
    
    auto newp = reinterpret_cast<std::uintptr_t>(p) + offset;
    return reinterpret_cast<T*>(roundUpPowerOfTwo(newp, align));
}
    
template<typename T>
inline T* pointerAdd(T* p, std::ptrdiff_t offset)
{
    return reinterpret_cast<T*>(reinterpret_cast<std::intptr_t>(p) + offset);
}
    
template<typename T, typename U>
inline std::ptrdiff_t pointerDistanceTo(T* p, U* q)
{
    return reinterpret_cast<std::intptr_t>(q) - reinterpret_cast<std::intptr_t>(p);
}
    
template<typename T>
inline bool isAligned(T* p)
{
    constexpr auto align = detail::alignment<T>();
    return (reinterpret_cast<std::uintptr_t>(p) & (align - 1)) == 0;
}
    
template<typename T, typename U, typename = std::enable_if_t<std::is_pointer_v<T>>>
inline T alignedCast(U* p)
{
    auto value = reinterpret_cast<std::uintptr_t>(p);
    assert(isAligned(p));
    assert(isAligned(reinterpret_cast<T>(p)));
    
    return reinterpret_cast<T>(value);
}

template<typename T>
inline T* roundUpPowerOfTwo(T* p, size_t align)
{
    assert(isValidAlignment(align));
    return alignedCast<T*>(
               reinterpret_cast<T*>(
                    reinterpret_cast<std::uintptr_t>(p + align - 1) & ~(align - 1)));
}
    
template<typename T>
inline T* roundDownPowerOfTwo(T* p, size_t align)
{
    assert(isValidAlignment(align));
    return alignedCast<T*>(
               reinterpret_cast<T*>(
                    reinterpret_cast<std::uintptr_t>(p) & ~(align - 1)));
}

} // namespace memory

#endif /* MEMORY_UTILS_H */
