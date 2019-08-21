//
//  bounded_allocator.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef BOUNDED_ALLOCATOR_H
#define BOUNDED_ALLOCATOR_H

#include "aligned_allocator.h"

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace memory
{
    
template<typename Allocator, std::uint32_t Tag = 0xDEADBEAFu>
class BoundedAllocator : protected AlignedAllocator<Allocator>
{
    using base_t = AlignedAllocator<Allocator>;
public:
    BoundedAllocator(Allocator& alloc)
        : base_t(alloc)
    {
    }
    
    void* malloc(std::size_t size)
    {
        // header + user memory + tag
        auto totalSize = sizeof(std::uint32_t) + size + sizeof(std::uint32_t);
        auto p = static_cast<std::uint32_t*>(base_t::malloc(totalSize, alignof(std::uint32_t)));
        *p = static_cast<std::uint32_t>(size);
        
        auto tagData = Tag;
        std::memcpy(pointerAdd(p, sizeof(std::uint32_t) + size), &tagData, sizeof(std::uint32_t));
        return &p[1];
    }
    
    void free(void* p)
    {
        if (p) {
            auto user = static_cast<char*>(p);
            auto userSize = alignedCast<std::uint32_t*>(p)[-1];
            auto tagData = Tag;
            assert(std::memcmp(user + userSize, &tagData, sizeof(Tag)) == 0);
            base_t::free(user - sizeof(std::uint32_t));
        }
    }
};

} // namespace memory

#endif /* BOUNDED_ALLOCATOR_H */
