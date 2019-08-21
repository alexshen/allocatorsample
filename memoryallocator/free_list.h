//
//  free_list.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef FREE_LIST_H
#define FREE_LIST_H

#include "memory_utils.h"

#include <cstddef>
#include <algorithm>

namespace memory
{
    
class FreeList
{
    struct Block
    {
        Block* next;
    };
public:
    static constexpr std::size_t minBlockSize = sizeof(Block);

    FreeList() = default;
    
    FreeList(void* beg, void* end, std::size_t size)
    {
        init(static_cast<char*>(beg), static_cast<char*>(end), size);
    }
    
    FreeList(const FreeList&) = delete;
    FreeList(FreeList&& rhs)
    {
        swap(rhs);
    }
    
    FreeList& operator =(const FreeList& rhs) = delete;
    FreeList& operator =(FreeList&& rhs)
    {
        swap(rhs);
        return *this;
    }
    
    void swap(FreeList& rhs)
    {
        std::swap(m_head, rhs.m_head);
    }
    
    bool empty() const { return !m_head; }

    void* malloc()
    {
        if (m_head) {
            auto next = m_head;
            m_head = m_head->next;
            return next;
        }
        return nullptr;
    }
    
    void free(void* p)
    {
        if (p) {
            auto block = static_cast<Block*>(p);
            block->next = m_head;
            m_head = block;
        }
    }
    
    static std::size_t adjustBlockSize(std::size_t n)
    {
        return roundUp(n, minBlockSize);
    }
private:
    void init(char* beg, char* end, std::size_t size)
    {
        assert(beg && end && beg < end);
        
        assert(size >= sizeof(Block));
        static_assert(isPowerOfTwo(sizeof(Block)));
        
        beg = align(beg, alignof(Block));
        assert(beg <= end);
        size = roundUpPowerOfTwo(size, sizeof(Block));
        
        auto cur = alignedCast<Block*>(beg);
        auto numBlocks = (end - beg) / size;
        if (numBlocks) {
            m_head = cur;
            for (std::size_t i = 0; i + 1 < numBlocks; ++i) {
                cur = cur->next = alignedCast<Block*>(pointerAdd(cur, size));
            }
            cur->next = nullptr;
        }
    }
    
    Block* m_head = nullptr;
};

} // namespace memory

#endif /* FREE_LIST_H */
