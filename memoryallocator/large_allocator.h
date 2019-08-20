//
//  memory.h
//  memoryallocator
//
//  Created by ashen on 2019/8/18.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef MEMORY_H
#define MEMORY_H

#pragma once

#include "rb_tree.h"
#include "list.h"
#include <cstddef>

namespace memory
{
    
class LargeAllocator
{
public:
    LargeAllocator(void* beg, void* end, std::size_t minBlockSize = 0);

    LargeAllocator(const LargeAllocator&) = delete;
    LargeAllocator& operator =(const LargeAllocator&) = delete;
    
    LargeAllocator(LargeAllocator&& rhs);
    LargeAllocator& operator =(LargeAllocator&& rhs);

    void swap(LargeAllocator& rhs);
    
    void* malloc(std::size_t size);
    void free(void* p);
private:
    void init(char* beg, char* end);
    
    struct Block : RbTreeNode, ListNode<Block>
    {
        std::size_t size : sizeof(std::size_t) * 8 - 1;
        std::size_t free : 1;

        std::size_t totalSize() const;
        void setTotalSize(std::size_t total);
        
        bool operator <(const Block& rhs) const;
    };
    
    // all the blocks, allocated or free, ordered by address
    List<Block> m_blocks;
    RbTree<Block> m_freeList;
    std::size_t m_minBlockSize;
};

} // namespace memory

#endif /* MEMORY_H */
