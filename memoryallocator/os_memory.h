//
//  os_memory.h
//  memoryallocator
//
//  Created by ashen on 2019/8/21.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef OS_MEMORY_H
#define OS_MEMORY_H

#include <cstddef>

namespace memory
{
    
std::size_t vmPageSize();
void* vmAllocate(std::size_t sizeBytes);
void vmDeallocate(void* p, std::size_t sizeBytes);
    
} // namespace memory

#endif /* OS_MEMORY_H */
