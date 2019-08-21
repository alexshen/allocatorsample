//
//  aligned_allocator.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef ALIGNED_ALLOCATOR_H
#define ALIGNED_ALLOCATOR_H

#include "memory_utils.h"

#include <cstddef>
#include <cassert>
#include <limits>

namespace memory
{
    
template<typename Allocator>
class AlignedAllocator
{
    static constexpr int MaxAlign = 256;
public:
    AlignedAllocator(Allocator& alloc)
        : m_allocator(&alloc)
    {}
    
    void* malloc(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
    {
        assert(alignment && isPowerOfTwo(alignment));
        assert(alignment < MaxAlign);
        
        auto p = static_cast<unsigned char*>(m_allocator->malloc(size + alignment));
        auto alignedP = roundUpPowerOfTwo(p + 1, alignment);
        // save the offset
        alignedP[-1] = alignment == MaxAlign ? 0 : alignedP - p;
        return alignedP;
    }
    
    void free(void* p)
    {
        if (p) {
            auto alignedP = static_cast<unsigned char*>(p);
            auto unalignedP = alignedP - (alignedP[-1] ? alignedP[-1] : MaxAlign);
            m_allocator->free(unalignedP);
        }
    }
private:
    Allocator* m_allocator;
};

} // namespace memory

#endif /* ALIGNED_ALLOCATOR_H */
