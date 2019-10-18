//
//  segregated_allocator.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef SEGREGATED_ALLOCATOR_H
#define SEGREGATED_ALLOCATOR_H

#include "free_list.h"
#include "list.h"
#include "os_memory.h"

#include <cstddef>
#include <algorithm>

namespace memory
{
    
template<std::size_t MaxBins>
class SegregatedAllocator
{
    static_assert(MaxBins > 0);
public:
    SegregatedAllocator(std::size_t minBinSize, std::size_t sizeStep)
    {
        assert(minBinSize > 0 && sizeStep > 0);
        m_minBinSize = FreeList::adjustBlockSize(minBinSize);
        m_sizeStep = FreeList::adjustBlockSize(sizeStep);
    }
    
    SegregatedAllocator(const SegregatedAllocator&) = delete;
    SegregatedAllocator(SegregatedAllocator&&) = delete;
    SegregatedAllocator& operator =(const SegregatedAllocator&) = delete;
    SegregatedAllocator& operator =(SegregatedAllocator&&) = delete;
    
    ~SegregatedAllocator()
    {
        for (auto& list : m_pageLists) {
            for (auto cur = list.first(); cur; ) {
                auto next = cur->next();
                vmDeallocate(cur, vmPageSize());
                cur = next;
            }
        }
    }
    
    std::size_t maxBinSize() const
    {
        return m_minBinSize + m_sizeStep * (MaxBins - 1);
    }
    
    void* malloc(std::size_t size)
    {
        auto bin = (std::max(size, m_minBinSize) - m_minBinSize + m_sizeStep - 1) / m_sizeStep;
        if (bin >= MaxBins) {
            return nullptr;
        }
        
        auto page = m_pageLists[bin].first();
        if (!page || page->freeList.empty()) {
            char* p = static_cast<char*>(vmAllocate(vmPageSize()));
            if (!p) {
                return nullptr;
            }
            page = new (p) Page;
            page->freeList = FreeList(p + sizeof(Page), p + vmPageSize(), m_minBinSize + bin * m_sizeStep);
            page->list = &m_pageLists[bin];
            m_pageLists[bin].addFirst(*page);
        }
        return page->freeList.malloc();
    }
    
    void free(void* p)
    {
        if (p) {
            auto page = alignedCast<Page*>(roundDownPowerOfTwo(p, vmPageSize()));
            bool wasEmpty = page->freeList.empty();
            page->freeList.free(p);
            if (wasEmpty && page != page->list->first()) {
                assert(page->list->first());
                page->list->remove(*page);
                page->list->insertAfter(*page, *page->list->first());
            }
        }
    }
private:
    struct Page : ListNode<Page>
    {
        FreeList freeList;
        List<Page>* list = nullptr;
    };
    
    List<Page> m_pageLists[MaxBins];
    std::size_t m_minBinSize;
    std::size_t m_sizeStep;
};

} // namespace memory

#endif /* SEGREGATED_ALLOCATOR_H */
