/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTLList_DEFINED
#define SkTLList_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkTInternalLList.h"
#include <new>
#include <utility>

/** Doubly-linked list of objects. The objects' lifetimes are controlled by the list. I.e. the
    the list creates the objects and they are deleted upon removal. This class block-allocates
    space for entries based on a param passed to the constructor.

    Elements of the list can be constructed in place using the following macros:
        SkNEW_INSERT_IN_LLIST_BEFORE(list, location, type_name, args)
        SkNEW_INSERT_IN_LLIST_AFTER(list, location, type_name, args)
    where list is a SkTLList<type_name>*, location is an iterator, and args is the paren-surrounded
    constructor arguments for type_name. These macros behave like addBefore() and addAfter().

    allocCnt is the number of objects to allocate as a group. In the worst case fragmentation
    each object is using the space required for allocCnt unfragmented objects.
*/
template <typename T, unsigned int N> class SkTLList {
private:
    struct Block;
    struct Node {
        SkAlignedSTStorage<1, T> fObj;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Node);
        Block* fBlock; // owning block.
    };
    typedef SkTInternalLList<Node> NodeList;

public:
    class Iter;

    // Having fCount initialized to -1 indicates that the first time we attempt to grab a free node
    // all the nodes in the pre-allocated first block need to be inserted into the free list. This
    // allows us to skip that loop in instances when the list is never populated.
    SkTLList() : fCount(-1) {}

    ~SkTLList() {
        this->validate();
        typename NodeList::Iter iter;
        Node* node = iter.init(fList, Iter::kHead_IterStart);
        while (node) {
            reinterpret_cast<T*>(node->fObj.get())->~T();
            Block* block = node->fBlock;
            node = iter.next();
            if (0 == --block->fNodesInUse) {
                for (unsigned int i = 0; i < N; ++i) {
                    block->fNodes[i].~Node();
                }
                if (block != &fFirstBlock) {
                    sk_free(block);
                }
            }
        }
    }

    /** Adds a new element to the list at the head. */
    template <typename... Args> T* addToHead(Args&&... args) {
        this->validate();
        Node* node = this->createNode();
        fList.addToHead(node);
        this->validate();
        return new (node->fObj.get())  T(std::forward<Args>(args)...);
    }

    /** Adds a new element to the list at the tail. */
    template <typename... Args> T* addToTail(Args&&... args) {
        this->validate();
        Node* node = this->createNode();
        fList.addToTail(node);
        this->validate();
        return new (node->fObj.get()) T(std::forward<Args>(args)...);
    }

    /** Adds a new element to the list before the location indicated by the iterator. If the
        iterator refers to a nullptr location then the new element is added at the tail */
    template <typename... Args> T* addBefore(Iter location, Args&&... args) {
        this->validate();
        Node* node = this->createNode();
        fList.addBefore(node, location.getNode());
        this->validate();
        return new (node->fObj.get()) T(std::forward<Args>(args)...);
    }

    /** Adds a new element to the list after the location indicated by the iterator. If the
        iterator refers to a nullptr location then the new element is added at the head */
    template <typename... Args> T* addAfter(Iter location, Args&&... args) {
        this->validate();
        Node* node = this->createNode();
        fList.addAfter(node, location.getNode());
        this->validate();
        return new (node->fObj.get()) T(std::forward<Args>(args)...);
    }

    /** Convenience methods for getting an iterator initialized to the head/tail of the list. */
    Iter headIter() const { return Iter(*this, Iter::kHead_IterStart); }
    Iter tailIter() const { return Iter(*this, Iter::kTail_IterStart); }

    T* head() { return Iter(*this, Iter::kHead_IterStart).get(); }
    T* tail() { return Iter(*this, Iter::kTail_IterStart).get(); }
    const T* head() const { return Iter(*this, Iter::kHead_IterStart).get(); }
    const T* tail() const { return Iter(*this, Iter::kTail_IterStart).get(); }

    void popHead() {
        this->validate();
        Node* node = fList.head();
        if (node) {
            this->removeNode(node);
        }
        this->validate();
    }

    void popTail() {
        this->validate();
        Node* node = fList.head();
        if (node) {
            this->removeNode(node);
        }
        this->validate();
    }

    void remove(T* t) {
        this->validate();
        Node* node = reinterpret_cast<Node*>(t);
        SkASSERT(reinterpret_cast<T*>(node->fObj.get()) == t);
        this->removeNode(node);
        this->validate();
    }

    void reset() {
        this->validate();
        Iter iter(*this, Iter::kHead_IterStart);
        while (iter.get()) {
            Iter next = iter;
            next.next();
            this->remove(iter.get());
            iter = next;
        }
        SkASSERT(0 == fCount || -1 == fCount);
        this->validate();
    }

    int count() const { return SkTMax(fCount ,0); }
    bool isEmpty() const { this->validate(); return 0 == fCount || -1 == fCount; }

    bool operator== (const SkTLList& list) const {
        if (this == &list) {
            return true;
        }
        // Call count() rather than use fCount because an empty list may have fCount = 0 or -1.
        if (this->count() != list.count()) {
            return false;
        }
        for (Iter a(*this, Iter::kHead_IterStart), b(list, Iter::kHead_IterStart);
             a.get();
             a.next(), b.next()) {
            SkASSERT(b.get()); // already checked that counts match.
            if (!(*a.get() == *b.get())) {
                return false;
            }
        }
        return true;
    }
    bool operator!= (const SkTLList& list) const { return !(*this == list); }

    /** The iterator becomes invalid if the element it refers to is removed from the list. */
    class Iter : private NodeList::Iter {
    private:
        typedef typename NodeList::Iter INHERITED;

    public:
        typedef typename INHERITED::IterStart IterStart;
        //!< Start the iterator at the head of the list.
        static const IterStart kHead_IterStart = INHERITED::kHead_IterStart;
        //!< Start the iterator at the tail of the list.
        static const IterStart kTail_IterStart = INHERITED::kTail_IterStart;

        Iter() {}

        Iter(const SkTLList& list, IterStart start = kHead_IterStart) {
            INHERITED::init(list.fList, start);
        }

        T* init(const SkTLList& list, IterStart start = kHead_IterStart) {
            return this->nodeToObj(INHERITED::init(list.fList, start));
        }

        T* get() { return this->nodeToObj(INHERITED::get()); }

        T* next() { return this->nodeToObj(INHERITED::next()); }

        T* prev() { return this->nodeToObj(INHERITED::prev()); }

        Iter& operator= (const Iter& iter) { INHERITED::operator=(iter); return *this; }

    private:
        friend class SkTLList;
        Node* getNode() { return INHERITED::get(); }

        T* nodeToObj(Node* node) {
            if (node) {
                return reinterpret_cast<T*>(node->fObj.get());
            } else {
                return nullptr;
            }
        }
    };

private:
    struct Block {
        int fNodesInUse;
        Node fNodes[N];
    };

    void delayedInit() {
        SkASSERT(-1 == fCount);
        fFirstBlock.fNodesInUse = 0;
        for (unsigned int i = 0; i < N; ++i) {
            fFreeList.addToHead(fFirstBlock.fNodes + i);
            fFirstBlock.fNodes[i].fBlock = &fFirstBlock;
        }
        fCount = 0;
        this->validate();
    }

    Node* createNode() {
        if (-1 == fCount) {
            this->delayedInit();
        }
        Node* node = fFreeList.head();
        if (node) {
            fFreeList.remove(node);
            ++node->fBlock->fNodesInUse;
        } else {
            // Should not get here when count == 0 because we always have the preallocated first
            // block.
            SkASSERT(fCount > 0);
            Block* block = reinterpret_cast<Block*>(sk_malloc_throw(sizeof(Block)));
            node = &block->fNodes[0];
            new (node) Node;
            node->fBlock = block;
            block->fNodesInUse = 1;
            for (unsigned int i = 1; i < N; ++i) {
                new (block->fNodes + i) Node;
                fFreeList.addToHead(block->fNodes + i);
                block->fNodes[i].fBlock = block;
            }
        }
        ++fCount;
        return node;
    }

    void removeNode(Node* node) {
        SkASSERT(node);
        fList.remove(node);
        reinterpret_cast<T*>(node->fObj.get())->~T();
        Block* block = node->fBlock;
        // Don't ever elease the first block, just add its nodes to the free list
        if (0 == --block->fNodesInUse && block != &fFirstBlock) {
            for (unsigned int i = 0; i < N; ++i) {
                if (block->fNodes + i != node) {
                    fFreeList.remove(block->fNodes + i);
                }
                block->fNodes[i].~Node();
            }
            sk_free(block);
        } else {
            fFreeList.addToHead(node);
        }
        --fCount;
        this->validate();
    }

    void validate() const {
#ifdef SK_DEBUG
        bool isEmpty = false;
        if (-1 == fCount) {
            // We should not yet have initialized the free list.
            SkASSERT(fFreeList.isEmpty());
            isEmpty = true;
        } else if (0 == fCount) {
            // Should only have the nodes from the first block in the free list.
            SkASSERT(fFreeList.countEntries() == N);
            isEmpty = true;
        }
        SkASSERT(isEmpty == fList.isEmpty());
        fList.validate();
        fFreeList.validate();
        typename NodeList::Iter iter;
        Node* freeNode = iter.init(fFreeList, Iter::kHead_IterStart);
        while (freeNode) {
            SkASSERT(fFreeList.isInList(freeNode));
            Block* block = freeNode->fBlock;
            // Only the first block is allowed to have all its nodes in the free list.
            SkASSERT(block->fNodesInUse > 0 || block == &fFirstBlock);
            SkASSERT((unsigned)block->fNodesInUse < N);
            int activeCnt = 0;
            int freeCnt = 0;
            for (unsigned int i = 0; i < N; ++i) {
                bool free = fFreeList.isInList(block->fNodes + i);
                bool active = fList.isInList(block->fNodes + i);
                SkASSERT(free != active);
                activeCnt += active;
                freeCnt += free;
            }
            SkASSERT(activeCnt == block->fNodesInUse);
            freeNode = iter.next();
        }

        int count = 0;
        Node* activeNode = iter.init(fList, Iter::kHead_IterStart);
        while (activeNode) {
            ++count;
            SkASSERT(fList.isInList(activeNode));
            Block* block = activeNode->fBlock;
            SkASSERT(block->fNodesInUse > 0 && (unsigned)block->fNodesInUse <= N);

            int activeCnt = 0;
            int freeCnt = 0;
            for (unsigned int i = 0; i < N; ++i) {
                bool free = fFreeList.isInList(block->fNodes + i);
                bool active = fList.isInList(block->fNodes + i);
                SkASSERT(free != active);
                activeCnt += active;
                freeCnt += free;
            }
            SkASSERT(activeCnt == block->fNodesInUse);
            activeNode = iter.next();
        }
        SkASSERT(count == fCount || (0 == count && -1 == fCount));
#endif
    }

    NodeList fList;
    NodeList fFreeList;
    Block    fFirstBlock;
    int fCount;

    SkTLList(const SkTLList&) = delete;
    SkTLList& operator=(const SkTLList&) = delete;
};

#endif
