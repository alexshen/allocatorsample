//
//  main.cpp
//  memoryallocator
//
//  Created by ashen on 2019/8/18.
//  Copyright © 2019 ashen. All rights reserved.
//

#include "large_allocator.h"
#include "segregated_allocator.h"
#include "aligned_allocator.h"
#include "bounded_allocator.h"
#include "free_list.h"
#include "rb_tree.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

using namespace std;

struct Foo : RbTreeNode
{
    int size;
    
    Foo() = default;
    
    Foo(int size) : size(size) {}
    
    bool operator <(const Foo& rhs) const
    {
        return size < rhs.size;
    }
};

template<typename It>
void check_equal(RbTree<Foo>& tree, It beg, It end)
{
    assert(equal(tree.begin(), tree.end(), beg, end, [] (auto const& a, Foo* b) {
        return a.size == b->size;
    }));
}

void testRbTree()
{
    RbTree<Foo> tree;
    vector<Foo> f;
    for (int i = 0; i < 45120; ++i) {
        f.emplace_back(rand() % 100);
    }
    //cout << &f[0] << endl;
    
    for (int i = 0; i < f.size(); ++i) {
        //cout << f[i].size << endl;
        tree.insert(f[i]);
    }
    
    vector<Foo*> pf;
    for (auto& e: f) {
        pf.push_back(&e);
    }
    sort(pf.begin(), pf.end(), [](Foo* a, Foo* b) {
        return *a < *b;
    });
    check_equal(tree, pf.begin(), pf.end());
    
    int i = 0;
    while (!tree.empty()) {
        auto index = rand() % (pf.size() - i) + i;
        //auto index = pf.size() - i - 1;
        auto p = pf[index];
        swap(pf[i], pf[index]);
        tree.remove(*p);
        ++i;
    }
}

int main(int argc, const char * argv[]) {
    constexpr int size = 1024*1024*10;
    auto buf = new char[size];
    
    {
        memory::LargeAllocator allocator(buf, buf + size);
        auto p = allocator.malloc(1 * 1024 * 1024);
        allocator.free(p);
        
        struct alignas(16) S {
            int a[10];
        };
        
        memory::BoundedAllocator boundedAlloc(allocator);
        memory::AlignedAllocator alignedAlloc(boundedAlloc);
        
        auto s = static_cast<S*>(alignedAlloc.malloc(sizeof(S), alignof(S)));
        memset(s + 1, 1, 4);
        alignedAlloc.free(s);
    }
    
    {
        memory::FreeList freeList(buf, buf + size, 12);
        freeList.free(freeList.malloc());
    }
    
    {
        memory::SegregatedAllocator<1> allocator(8, 8);
        allocator.free(allocator.malloc(7));
        allocator.malloc(9);
    }

    delete[] buf;
}
