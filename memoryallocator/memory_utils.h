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

namespace memory
{
    
namespace detail
{

constexpr bool isPowerOfTwo(std::size_t n)
{
    return n & ~(n - 1);
}
    
} // namespace detail
    
inline std::size_t roundUpPowerOfTwo(std::size_t size, std::size_t align)
{
    assert(align && detail::isPowerOfTwo(align));
    return (size + align - 1) & ~(align - 1);
}
    
template<typename T>
inline T* align(T* p, std::size_t align, std::size_t offset = 0)
{
    assert(align && detail::isPowerOfTwo(align));
    
    auto newp = reinterpret_cast<std::uintptr_t>(p) + offset;
    return reinterpret_cast<T*>(roundUpPowerOfTwo(newp, align));
}

} // namespace memory

#endif /* MEMORY_UTILS_H */
