//
//  list.h
//  memoryallocator
//
//  Created by ashen on 2019/8/20.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef LIST_H
#define LIST_H

#include <algorithm>

template<typename T>
class List;

template<typename T>
class ListNode
{
    friend class List<T>;
public:
    T* prev() const { return m_prev; }
    T* next() const { return m_next; }
private:
    void setPrev(T* p) { m_prev = p; }
    void setNext(T* p) { m_next = p; }
    
    T* m_prev = nullptr;
    T* m_next = nullptr;
};

template<typename T>
class List
{
public:
    List() = default;

    List(const List&) = delete;
    List& operator =(const List&) = delete;
    
    List(List&& rhs)
    {
        swap(rhs);
    }
    
    List& operator =(List&& rhs)
    {
        swap(rhs);
        return *this;
    }
    
    void swap(List& rhs)
    {
        std::swap(m_head, rhs.m_head);
        std::swap(m_tail, rhs.m_tail);
    }

    T* first() { return m_head; }
    T* last() { return m_tail; }
    
    const T* first() const { return m_head; }
    const T* last() const { return m_tail; }
    
    void addFirst(T& node)
    {
        node.setNext(m_head);
        if (m_head) {
            m_head->setPrev(&node);
        } else {
            m_tail = &node;
        }
        m_head = &node;
    }
    
    void addLast(T& node)
    {
        node.setPrev(m_tail);
        if (m_tail) {
            m_tail->setNext(&node);
        } else {
            m_head = &node;
        }
        m_tail = &node;
    }
    
    void insertAfter(T& node, T& after)
    {
        assert(!node.prev() && !node.next());
        
        if (after.next()) {
            after.next()->setPrev(&node);
            node.setNext(after.next());
            node.setPrev(&after);
            after.setNext(&node);
        } else {
            addLast(node);
        }
    }
    
    void insertBefore(T& node, T& before)
    {
        assert(!node.prev() && !node.next());
        
        if (before.prev()) {
            before.prev()->m_next = &node;
            node.setPrev(before.prev());
            node.setNext(&before);
            before.setPrev(&node);
        } else {
            addFirst(node);
        }
    }
    
    void remove(T& node)
    {
        assert(node.prev() || node.next());
        
        if (!node.prev()) {
            m_head = node.next();
        } else {
            node.prev()->setNext(node.next());
        }
        
        if (!node.next()) {
            m_tail = node.prev();
        } else {
            node.next()->setPrev(node.prev());
        }
        
        node.setPrev(nullptr);
        node.setNext(nullptr);
    }
private:
    T* m_head = nullptr;
    T* m_tail = nullptr;
};

#endif /* LIST_H */
