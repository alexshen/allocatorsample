//
//  os_memory.cpp
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#include "os_memory.h"
#include <cassert>

#ifdef __APPLE__
#  include <sys/mman.h>
#  include <unistd.h>
#endif

namespace memory
{
    
#ifdef __APPLE__

std::size_t vmPageSize()
{
    static const std::size_t pageSize = sysconf(_SC_PAGE_SIZE);
    return pageSize;
}
    
void* vmAllocate(std::size_t sizeBytes)
{
    assert(sizeBytes);
    void* p = mmap(nullptr, sizeBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return p != MAP_FAILED ? p : nullptr;
}
    
void vmDeallocate(void* p, std::size_t sizeBytes)
{
    assert(sizeBytes);
    auto res = munmap(p, sizeBytes);
    assert(!res);
}

#endif
    
} // namespace memory
