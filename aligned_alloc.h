//
//  aligned_alloc.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef ALIGNED_ALLOC_H
#define ALIGNED_ALLOC_H

#include "memory_utils.h"

#include <cstddef>
#include <cassert>
#include <limits>

namespace memory
{
    
constexpr std::size_t MaxAlign = 256;

constexpr std::size_t calcAlignedAllocSize(std::size_t size, std::size_t alignment)
{
    assert(alignment <= MaxAlign && isValidAlignment(alignment));
    // reserve enough space for the offset and alignment
    return size + alignment;
}
    
inline void* adjustForAlignedAlloc(void* p, std::size_t alignment)
{
    assert(p);
    assert(isValidAlignment(alignment) && alignment <= MaxAlign);
    
    auto unaligned = static_cast<unsigned char*>(p);
    auto alignedP = roundUpPowerOfTwo(unaligned + 1, alignment);
    // save the offset
    alignedP[-1] = alignment == MaxAlign ? 0 : alignedP - unaligned;
    return alignedP;
}
    
inline void* getUnalignedAlloc(void* p)
{
    assert(p);
    
    auto unaligned = static_cast<unsigned char*>(p);
    return unaligned - unaligned[-1];
}
    
} // namespace memory

#endif /* ALIGNED_ALLOC_H */
