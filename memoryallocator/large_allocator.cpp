//
//  memory.cpp
//  memoryallocator
//
//  Created by ashen on 2019/8/20.
//  Copyright © 2019 ashen. All rights reserved.
//

#include "large_allocator.h"
#include "aligned_alloc.h"
#include "memory_utils.h"

#include <new>
#include <cassert>

namespace memory
{
    
LargeAllocator::LargeAllocator(void* beg, void* end, std::size_t minBlockSize)
    : m_minBlockSize(roundUpPowerOfTwo(minBlockSize, alignof(Block)))
{
    init((char*)beg, (char*)end);
}

LargeAllocator::LargeAllocator(LargeAllocator&& rhs)
{
    swap(rhs);
}

LargeAllocator& LargeAllocator::operator =(LargeAllocator&& rhs)
{
    swap(rhs);
    return *this;
}

void LargeAllocator::swap(LargeAllocator& rhs)
{
    std::swap(m_minBlockSize, rhs.m_minBlockSize);
    m_blocks.swap(rhs.m_blocks);
    m_freeList.swap(rhs.m_freeList);
}

void* LargeAllocator::malloc(std::size_t size, std::size_t alignment)
{
    assert(size);
    assert(isValidAlignment(alignment));
    
    alignment = std::max(alignment, alignof(Block));
    
    Block targetBlock;
    // need to take into account Block alignment
    auto payloadSize = roundUpPowerOfTwo(size, alignof(Block));
    targetBlock.size = calcAlignedAllocSize(payloadSize, alignment);
    if (auto it = m_freeList.lowerBound(targetBlock); it != m_freeList.end()) {
        m_freeList.remove(*it);
        
        auto& block = const_cast<Block&>(*it);
        block.free = false;
        
        auto minSizeForSplit = targetBlock.size + sizeof(Block) + m_minBlockSize;
        // can split
        if (block.size >= minSizeForSplit) {
            auto oldSize = block.size;
            block.size = targetBlock.size;
            
            auto next = new (pointerAdd(&block, block.totalSize())) Block;
            next->size = oldSize - targetBlock.size - sizeof(Block);
            next->free = true;

            m_blocks.insertAfter(*next, block);
            m_freeList.insert(*next);
        }

        return adjustForAlignedAlloc(pointerAdd(&block, sizeof(Block)), alignment);
    }
    return nullptr;
}

void LargeAllocator::free(void* p)
{
    if (p) {
        auto block = alignedCast<Block*>(getUnalignedAlloc(p)) - 1;
        block->free = true;
        
        // coalesce with the previous or the next block if possible
        if (auto prev = block->prev(); prev && prev->free) {
            m_freeList.remove(*prev);
            m_blocks.remove(*block);
            prev->size += block->totalSize();
            block = prev;
        }
        
        if (auto next = block->next(); next && next->free) {
            m_freeList.remove(*next);
            m_blocks.remove(*next);
            block->size += next->totalSize();
        }
        
        m_freeList.insert(*block);
    }
}

void LargeAllocator::init(char* beg, char* end)
{
    assert(beg && end && beg <= end);
    
    beg = align(beg, alignof(Block));
    if (beg + sizeof(Block) <= end) {
        auto block = new (beg) Block;
        block->free = true;
        block->setTotalSize(end - beg);

        m_freeList.insert(*block);
        m_blocks.addFirst(*block);
    }
}

void LargeAllocator::Block::setTotalSize(std::size_t total)
{
    assert(total >= sizeof(Block));
    size = total - sizeof(Block);
}
    
std::size_t LargeAllocator::Block::totalSize() const
{
    return sizeof(Block) + size;
}
    
bool LargeAllocator::Block::operator <(const Block& rhs) const
{
    return size < rhs.size;
}
    
} // namespace memory
