//
//  rb_tree.h
//  memoryallocator
//
//  Created by ashen on 2019/8/18.
//  Copyright © 2019 ashen. All rights reserved.
//

#ifndef RB_TREE_H
#define RB_TREE_H

#include <type_traits>
#include <cstdint>
#include <functional>
#include <iterator>
#include <algorithm>

#pragma once

template<typename Tree, typename Key, typename Comp>
class RbTreeIterator;

template<typename Key, typename Comp>
class RbTree;

class RbTreeNode
{
    template<typename Key, typename Comp>
    friend class RbTree;
    
    template<typename Tree, typename Key, typename Comp>
    friend class RbTreeIterator;

    enum Color {
        red,
        black,
        mask = 0x1
    };
protected:
    RbTreeNode() = default;
private:
    RbTreeNode* parent() const
    {
        return reinterpret_cast<RbTreeNode*>(
                    reinterpret_cast<std::uintptr_t>(m_parent) & ~Color::mask);
    }
    
    RbTreeNode* left() const { return m_left; }
    RbTreeNode* right() const { return m_right; }
    
    RbTreeNode* minimum()
    {
        auto node = this;
        while (node->left()) {
            node = node->left();
        }
        return node;
    }
    
    RbTreeNode* maximum()
    {
        auto node = this;
        while (node->right()) {
            node = node->right();
        }
        return node;
    }
    
    RbTreeNode* successor()
    {
        if (right()) {
            return right()->minimum();
        }
        
        auto cur = this;
        auto parent = cur->parent();
        while (cur == parent->right()) {
            cur = parent;
            parent = parent->parent();
        }
        // cur->right == parent only when this is the rightmost node
        return cur->right() != parent ? parent : cur;
    }
    
    RbTreeNode* predecessor()
    {
        if (left()) {
            return left()->maximum();
        }
        
        auto cur = this;
        auto parent = this->parent();
        while (cur == parent->left()) {
            cur = parent;
            parent = parent->parent();
        }
        // cur->left == parent only when this is the leftmost node
        return cur->left() != parent ? parent : cur;
    }
    
    void setLeft(RbTreeNode* left) { m_left = left; }
    void setRight(RbTreeNode* right) { m_right = right; }
    
    void setParent(RbTreeNode* parent)
    {
        assert(parent != this);
        m_parent = reinterpret_cast<RbTreeNode*>(
                        reinterpret_cast<std::uintptr_t>(parent) | color());
    }
    
    void reset()
    {
        m_parent = m_left = m_right = nullptr;
    }
    
    Color color() const
    {
        return static_cast<Color>(
                    reinterpret_cast<std::uintptr_t>(m_parent) & Color::mask);
    }
    
    void setColor(Color color)
    {
        assert(color <= Color::black);
        m_parent = reinterpret_cast<RbTreeNode*>(
                     reinterpret_cast<std::uintptr_t>(parent()) | color);
    }
    
    RbTreeNode* m_parent = nullptr;
    RbTreeNode* m_left = nullptr;
    RbTreeNode* m_right = nullptr;
};

template<typename Key, typename Comp = std::less<Key>>
class RbTree : protected Comp
{
    static_assert(std::is_base_of_v<RbTreeNode, Key>, "Key must derive from RbTreeNode");
public:
    template<typename T, typename U, typename V>
    friend class RbTreeIterator;

    using iterator = RbTreeIterator<RbTree, Key, Comp>;
    using const_iterator = RbTreeIterator<RbTree, const Key, Comp>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;

    RbTree(const Comp& comp = Comp())
        : Comp(comp)
    {
        m_sentinel.setColor(RbTreeNode::black);
        m_sentinel.setLeft(&m_sentinel);
        m_sentinel.setRight(&m_sentinel);
    }
    
    RbTree(const RbTree&) = delete;
    RbTree(RbTree&& rhs)
    {
        swap(rhs);
    }
    
    RbTree& operator =(const RbTree&) = delete;
    RbTree& operator =(RbTree&& rhs)
    {
        swap(rhs);
        return *this;
    }
    
    void swap(RbTree& rhs)
    {
        std::swap(m_sentinel, rhs.m_sentinel);
        // fix up the parents of the root nodes
        if (root()) {
            root()->setParent(&rhs.m_sentinel);
        }
        if (rhs.root()) {
            rhs.root()->setParent(&m_sentinel);
        }
    }
    
    iterator begin() { return { *this, *leftmost() }; }
    const_iterator begin() const { return { const_cast<RbTree&>(*this), *leftmost() }; }
    
    reverse_iterator rbegin() { return end(); }
    reverse_const_iterator rbegin() const { return end(); }
    
    iterator end() { return { *this, m_sentinel }; }
    const_iterator end() const { return { const_cast<RbTree&>(*this), m_sentinel }; }
    
    reverse_iterator rend() { return begin(); }
    reverse_const_iterator rend() const { return begin(); }
    
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    
    reverse_const_iterator rcbegin() const { return end(); }
    reverse_const_iterator rcend() const { return begin(); }

    bool empty() const { return leftmost() == &m_sentinel; }
    
    void insert(Key& node)
    {
        assert(!node.parent());
        
        auto parent = &m_sentinel;
        auto cur = root();
        
        bool smaller = false;
        while (cur) {
            parent = cur;
            smaller = compare(node, static_cast<const Key&>(*cur));
            if (smaller) {
                cur = cur->left();
            } else {
                cur = cur->right();
            }
        }
        if (parent == &m_sentinel) {
            setRoot(&node);
            setLeftMost(&node);
            setRightMost(&node);
        } else {
            if (parent == leftmost() && smaller) {
                setLeftMost(&node);
            } else if (parent == rightmost() && !smaller) {
                setRightMost(&node);
            }
            if (smaller) {
                parent->setLeft(&node);
            } else {
                parent->setRight(&node);
            }
        }
        node.setParent(parent);
        node.setColor(RbTreeNode::red);

        insertFixup(&node);
        assert(node.parent());
        validate();
    }
    
    void remove(const Key& node)
    {
        assert(node.parent());
        
        // the node to delete or to replace `node'
        RbTreeNode* candidate;
        if (!node.left() || !node.right()) {
            candidate = const_cast<Key*>(&node);
        } else {
            candidate = const_cast<Key&>(node).successor();
        }
        RbTreeNode* child;
        if (candidate->left()) {
            child = candidate->left();
        } else {
            child = candidate->right();
        }
        if (candidate == rightmost()) {
            setRightMost(candidate->predecessor());
        }
        if (candidate == leftmost()) {
            setLeftMost(candidate->successor());
        }
        
        // reparent the child
        if (child && candidate != node.right()) {
            child->setParent(candidate->parent());
        }
        auto childParent = candidate->parent();
        bool isRoot = &node == root();
        
        if (candidate == root()) {
            setRoot(child);
        } else {
            if (isRoot){
                setRoot(candidate);
            }
            if (candidate == candidate->parent()->left()) {
                candidate->parent()->setLeft(child);
            } else if (candidate != node.right()) {
                candidate->parent()->setRight(child);
            }
        }

        auto candidateColor = candidate->color();
        // node has two children
        if (candidate != &node) {
            candidate->setLeft(node.left());
            if (candidate != node.right()) {
                candidate->setRight(node.right());
                node.right()->setParent(candidate);
            } else {
                childParent = candidate;
            }
            candidate->setParent(node.parent());
            candidate->setColor(node.color());
            node.left()->setParent(candidate);

            if (!isRoot) {
                if (&node == node.parent()->left()) {
                    node.parent()->setLeft(candidate);
                } else {
                    node.parent()->setRight(candidate);
                }
            }
        }
        if (candidateColor == RbTreeNode::black) {
            removeFixup(child, childParent);
        }
        const_cast<Key&>(node).reset();
        validate();
    }
    
    const_iterator lowerBound(const Key& key) const
    {
        const RbTreeNode* res = nullptr;
        auto cur = root();
        while (cur) {
            if (compare(static_cast<const Key&>(*cur), key)) {
                cur = cur->right();
            }  else {
                res = cur;
                cur = cur->left();
            }
        }
        return { *this, res ? *res : m_sentinel };
    }
    
    const_iterator find(const Key& key) const
    {
        const RbTreeNode* res = nullptr;
        auto cur = root();
        while (cur) {
            if (compare(static_cast<const Key&>(*cur), key)) {
                cur = cur->right();
            } else if (compare(key, static_cast<const Key&>(*cur))) {
                cur = cur->left();
            } else {
                res = cur;
                break;
            }
        }
        return { *this, res ? res : m_sentinel };
    }
private:
    bool compare(const Key& a, const Key& b) const
    {
        return static_cast<const Comp&>(*this)(a, b);
    }
    
    void insertFixup(RbTreeNode* cur)
    {
        while (cur->parent()->color() == RbTreeNode::red) {
            assert(cur->color() == RbTreeNode::red);
            auto parent = cur->parent();
            auto grandParent = parent->parent();
            if (parent == grandParent->left()) {
                auto uncle = grandParent->right();
                if (uncle && uncle->color() == RbTreeNode::red) {
                    parent->setColor(RbTreeNode::black);
                    uncle->setColor(RbTreeNode::black);
                    grandParent->setColor(RbTreeNode::red);
                    cur = grandParent;
                } else {
                    if (cur == parent->right()) {
                        cur = parent;
                        leftRotate(cur);
                        parent = cur->parent();
                        grandParent = parent->parent();
                    }
                    parent->setColor(RbTreeNode::black);
                    grandParent->setColor(RbTreeNode::red);
                    rightRotate(grandParent);
                }
            } else {
                auto uncle = grandParent->left();
                if (uncle && uncle->color() == RbTreeNode::red) {
                    parent->setColor(RbTreeNode::black);
                    uncle->setColor(RbTreeNode::black);
                    grandParent->setColor(RbTreeNode::red);
                    cur = grandParent;
                } else {
                    if (cur == parent->left()) {
                        cur = parent;
                        rightRotate(cur);
                        parent = cur->parent();
                        grandParent = parent->parent();
                    }
                    parent->setColor(RbTreeNode::black);
                    grandParent->setColor(RbTreeNode::red);
                    leftRotate(grandParent);
                }
            }
        }
        root()->setColor(RbTreeNode::black);
    }

    void removeFixup(RbTreeNode* cur, RbTreeNode* parent)
    {
        while (cur != root() && (!cur || cur->color() == RbTreeNode::black)) {
            if (cur == parent->left()) {
                auto sibling = parent->right();
                assert(sibling);
                if (sibling->color() == RbTreeNode::red) {
                    sibling->setColor(RbTreeNode::black);
                    parent->setColor(RbTreeNode::red);
                    leftRotate(parent);
                    sibling = parent->right();
                }
                if ((!sibling->left() || sibling->left()->color() == RbTreeNode::black) &&
                    (!sibling->right() || sibling->right()->color() == RbTreeNode::black)) {
                    sibling->setColor(RbTreeNode::red);
                    cur = parent;
                    parent = cur->parent();
                } else {
                    if (!sibling->right() || sibling->right()->color() == RbTreeNode::black) {
                        if (sibling->left()) {
                            sibling->left()->setColor(RbTreeNode::black);
                        }
                        sibling->setColor(RbTreeNode::red);
                        rightRotate(sibling);
                        sibling = parent->right();
                    }
                    sibling->setColor(parent->color());
                    parent->setColor(RbTreeNode::black);
                    if (sibling->right()) {
                        sibling->right()->setColor(RbTreeNode::black);
                    }
                    leftRotate(parent);
                    cur = root();
                }
            } else {
                auto sibling = parent->left();
                assert(sibling);
                if (sibling->color() == RbTreeNode::red) {
                    sibling->setColor(RbTreeNode::black);
                    parent->setColor(RbTreeNode::red);
                    rightRotate(parent);
                    sibling = parent->left();
                }
                if ((!sibling->left() || sibling->left()->color() == RbTreeNode::black) &&
                    (!sibling->right() || sibling->right()->color() == RbTreeNode::black)) {
                    sibling->setColor(RbTreeNode::red);
                    cur = parent;
                    parent = cur->parent();
                } else {
                    if (!sibling->left() || sibling->left()->color() == RbTreeNode::black) {
                        if (sibling->right()) {
                            sibling->right()->setColor(RbTreeNode::black);
                        }
                        sibling->setColor(RbTreeNode::red);
                        leftRotate(sibling);
                        sibling = parent->left();
                    }
                    sibling->setColor(parent->color());
                    parent->setColor(RbTreeNode::black);
                    if (sibling->left()) {
                        sibling->left()->setColor(RbTreeNode::black);
                    }
                    rightRotate(parent);
                    cur = root();
                }
            }
        }
        if (cur) {
            cur->setColor(RbTreeNode::black);
        }
    }
    
    void leftRotate(RbTreeNode* node)
    {
        if (!node->right()) { return; }
        
        auto right = node->right();
        node->setRight(right->left());
        if (right->left()) {
            right->left()->setParent(node);
        }
        right->setParent(node->parent());
        if (root() != node) {
            if (node == node->parent()->left()) {
                node->parent()->setLeft(right);
            } else {
                node->parent()->setRight(right);
            }
        } else {
            setRoot(right);
        }
        node->setParent(right);
        right->setLeft(node);
    }
    
    void rightRotate(RbTreeNode* node)
    {
        if (!node->left()) { return; }
        
        auto left = node->left();
        node->setLeft(left->right());
        if (left->right()) {
            left->right()->setParent(node);
        }
        left->setParent(node->parent());
        if (root() != node) {
            if (node == node->parent()->left()) {
                node->parent()->setLeft(left);
            } else {
                node->parent()->setRight(left);
            }
        } else {
            setRoot(left);
        }
        node->setParent(left);
        left->setRight(node);
    }

    void validate()
    {
#ifndef NDEBUG
        assert(!root() || root()->color() == RbTreeNode::black);
        assert(!root() || root()->parent() == &m_sentinel);
        validate(root());
#endif
    }
    
#ifndef NDEBUG
    int validate(RbTreeNode* key) const
    {
        if (!key) { return 0; }
        
        if (key->left()) {
            assert(key->left()->parent() == key);
        }
        if (key->right()) {
            assert(key->right()->parent() == key);
        }
        auto lbh = validate(key->left());
        auto rbh = validate(key->right());
        assert(lbh == rbh);
        return lbh + key->color() == RbTreeNode::black;
    }
#endif
    
    RbTreeNode* root() const { return m_sentinel.parent(); }
    void setRoot(RbTreeNode* root) { m_sentinel.setParent(root); }
    
    RbTreeNode* leftmost() const { return m_sentinel.left(); }
    void setLeftMost(RbTreeNode* node) { m_sentinel.setLeft(node); }
    
    RbTreeNode* rightmost() const { return m_sentinel.right(); }
    void setRightMost(RbTreeNode* node) { m_sentinel.setRight(node); }
    
    RbTreeNode m_sentinel;
};

template<typename Tree, typename Key, typename Comp>
class RbTreeIterator
{
public:
    using value_type = std::remove_const_t<Key>;
    using reference = Key&;
    using pointer = Key*;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    
private:
    friend Tree;
    
    template<typename T, typename U, typename V>
    friend class RbTreeIterator;
    
    RbTreeIterator(const Tree& tree, const RbTreeNode& node)
        : m_tree(const_cast<Tree*>(&tree))
        , m_node(const_cast<RbTreeNode*>(&node))
    {
    }
    
    RbTreeIterator(Tree& tree, RbTreeNode& node)
        : m_tree(&tree)
        , m_node(&node)
    {
    }
public:
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Key*>>>
    RbTreeIterator(const RbTreeIterator<Tree, T, Comp>& rhs)
        : RbTreeIterator(*rhs.m_tree, *rhs.m_node)
    {
    }
    
    RbTreeIterator(const RbTreeIterator&) = default;
    RbTreeIterator(RbTreeIterator&&) = default;
    
    RbTreeIterator& operator =(const RbTreeIterator&) = default;
    RbTreeIterator& operator =(RbTreeIterator&&) = default;
public:
    RbTreeIterator& operator++()
    {
        assert(m_node != &m_tree->m_sentinel);
        m_node = m_node->successor();
        return *this;
    }

    RbTreeIterator operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return *this;
    }
    
    RbTreeIterator& operator--()
    {
        assert(!m_tree->empty());
        m_node = m_node == &m_tree->m_sentinel ? m_tree->rightmost() : m_node->predecessor();
        return *this;
    }
    
    RbTreeIterator operator--(int)
    {
        auto tmp = *this;
        --*this;
        return *this;
    }
    
    reference operator *() const
    {
        assert(m_node);
        return static_cast<reference>(*m_node);
    }
    
    pointer operator ->() const
    {
        return &**this;
    }
    
    bool operator ==(const RbTreeIterator& rhs) const
    {
        assert(m_tree == rhs.m_tree);
        return m_node == rhs.m_node;
    }
    
    bool operator !=(const RbTreeIterator& rhs) const
    {
        return !(*this == rhs);
    }
private:
    Tree* m_tree;
    RbTreeNode* m_node;
};

#endif /* RB_TREE_H */
