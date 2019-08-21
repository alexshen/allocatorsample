//
//  bounded_allocator.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef BOUNDED_ALLOCATOR_H
#define BOUNDED_ALLOCATOR_H

#include "memory_utils.h"

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace memory
{
    
template<typename Allocator, std::uint32_t Tag = 0xDEADBEAFu>
class BoundedAllocator
{
    struct Header
    {
        std::uint32_t userSize;
        std::uint32_t offset;
    };
public:
    BoundedAllocator(Allocator& alloc)
        : m_allocator(&alloc)
    {
    }
    
    void* malloc(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
    {
        auto maxAlign = std::max(alignment, alignof(std::uint32_t));
        // header + alignment padding + user memory + tag
        auto totalSize = roundUpPowerOfTwo(sizeof(Header), maxAlign) + size + sizeof(std::uint32_t);
        auto p = static_cast<char*>(m_allocator->malloc(totalSize, maxAlign));
        auto user = alignedCast<Header*>(p + sizeof(Header));
        user[-1] = {
            static_cast<std::uint32_t>(size),
            static_cast<std::uint32_t>(pointerDistanceTo(p, user))
        };
        
        auto tagData = Tag;
        std::memcpy(pointerAdd(user, size), &tagData, sizeof(std::uint32_t));
        return user;
    }
    
    void free(void* p)
    {
        if (p) {
            auto user = static_cast<char*>(p);
            const auto& header = alignedCast<const Header*>(p)[-1];
            auto tagData = Tag;
            // check if the tag was overwritten
            assert(std::memcmp(user + header.userSize, &tagData, sizeof(Tag)) == 0);
            m_allocator->free(user - header.offset);
        }
    }
private:
    Allocator* m_allocator;
};

} // namespace memory

#endif /* BOUNDED_ALLOCATOR_H */
