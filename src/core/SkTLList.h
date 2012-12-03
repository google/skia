/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTInternalLList.h"
#include "SkTemplates.h"

/** Doubly-linked list of objects. The objects' lifetimes are controlled by the list. I.e. the
    the list creates the objects and they are deleted upon removal. */
template <typename T>
class SkTLList : public SkNoncopyable {
private:
    struct Block;
    struct Node {
        SkAlignedSTStorage<1, T> fObj;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Node);
        Block* fBlock;
    };
    typedef SkTInternalLList<Node> NodeList;

public:
    /** allocCnt is the number of objects to allocate as a group. In the worst case fragmentation
        each object is using the space required for allocCnt unfragmented objects. */
    SkTLList(int allocCnt = 1) : fCount(0), fAllocCnt(allocCnt) {
        SkASSERT(allocCnt > 0);
        this->validate();
    }

    ~SkTLList() {
        this->validate();
        NodeList::Iter iter;
        Node* node = iter.init(fList, Iter::kHead_IterStart);
        while (NULL != node) {
            reinterpret_cast<T*>(node->fObj.get())->~T();
            Block* block = node->fBlock;
            node = iter.next();
            if (0 == --block->fNodesInUse) {
                sk_free(block);
                for (int i = 0; i < fAllocCnt; ++i) {
                    block->fNodes[i].~Node();
                }
            }
        }
    }
    
    void addToHead(const T& t) {
        this->validate();
        Node* node = this->createNode();
        fList.addToHead(node);
        SkNEW_PLACEMENT_ARGS(node->fObj.get(), T, (t));
        this->validate();
    }

    void addToTail(const T& t) {
        this->validate();
        Node* node = this->createNode();
        fList.addToTail(node);
        SkNEW_PLACEMENT_ARGS(node->fObj.get(), T, (t));
        this->validate();
    }

    void popHead() {
        this->validate();
        Node* node = fList.head();
        if (NULL != node) {
            this->removeNode(node);
        }
        this->validate();
    }

    void popTail() {
        this->validate();
        Node* node = fList.head();
        if (NULL != node) {
            this->removeNode(node);
        }
        this->validate();
    }

    void remove(T* t) {
        this->validate();
        Node* node = reinterpret_cast<Node*>(t);
        SkASSERT(node->fObj.get() == t);
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
        SkASSERT(0 == fCount);
        this->validate();
    }

    int count() const { return fCount; }
    bool isEmpty() const { this->validate(); return NULL == fCount; }

    bool operator== (const SkTLList& list) const {
        if (this == &list) {
            return true;
        }
        if (fCount != list.fCount) {
            return false;
        }
        for (Iter a(*this, Iter::kHead_IterStart), b(list, Iter::kHead_IterStart);
             a.get();
             a.next(), b.next()) {
            SkASSERT(NULL != b.get());
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

        Iter() : INHERITED() {}

        Iter(const SkTLList& list, IterStart start) : INHERITED() {
            INHERITED::init(list.fList, start);
        }

        T* init(const SkTLList& list, IterStart start) {
            return this->nodeToObj(INHERITED::init(list.fList, start));
        }

        T* get() { return this->nodeToObj(INHERITED::get()); }

        T* next() { return this->nodeToObj(INHERITED::next()); }

        T* prev() { return this->nodeToObj(INHERITED::prev()); }

        Iter& operator= (const Iter& iter) { INHERITED::operator=(iter); return *this; }

    private:
        T* nodeToObj(Node* node) {
            if (NULL != node) {
                return reinterpret_cast<T*>(node->fObj.get());
            } else {
                return NULL;
            }
        }
    };

private:
    struct Block {
        int fNodesInUse;
        Node fNodes[1];
    };

    size_t blockSize() const { return sizeof(Block) + sizeof(Node) * (fAllocCnt-1); }

    Node* createNode() {
        Node* node;
        if (node = fFreeList.head()) {
            fFreeList.remove(node);
            ++node->fBlock->fNodesInUse;
        } else {
            Block* block = reinterpret_cast<Block*>(sk_malloc_flags(this->blockSize(), 0));
            node = &block->fNodes[0];
            SkNEW_PLACEMENT(node, Node);
            node->fBlock = block;
            block->fNodesInUse = 1;
            for (int i = 1; i < fAllocCnt; ++i) {
                SkNEW_PLACEMENT(block->fNodes + i, Node);
                fFreeList.addToHead(block->fNodes + i);
                block->fNodes[i].fBlock = block;
            }
        }
        ++fCount;
        return node;
    }

    void removeNode(Node* node) {
        SkASSERT(NULL != node);
        fList.remove(node);
        ((T*)node->fObj.get())->~T();
        if (0 == --node->fBlock->fNodesInUse) {
            Block* block = node->fBlock;
            for (int i = 0; i < fAllocCnt; ++i) {
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
        SkASSERT((0 == fCount) == fList.isEmpty());
        SkASSERT((0 != fCount) || fFreeList.isEmpty());

        fList.validate();
        fFreeList.validate();
        NodeList::Iter iter;
        Node* freeNode = iter.init(fFreeList, Iter::kHead_IterStart);
        while (freeNode) {
            SkASSERT(fFreeList.isInList(freeNode));
            Block* block = freeNode->fBlock;
            SkASSERT(block->fNodesInUse > 0 && block->fNodesInUse < fAllocCnt);

            int activeCnt = 0;
            int freeCnt = 0;
            for (int i = 0; i < fAllocCnt; ++i) {
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
            SkASSERT(block->fNodesInUse > 0 && block->fNodesInUse <= fAllocCnt);

            int activeCnt = 0;
            int freeCnt = 0;
            for (int i = 0; i < fAllocCnt; ++i) {
                bool free = fFreeList.isInList(block->fNodes + i);
                bool active = fList.isInList(block->fNodes + i);
                SkASSERT(free != active);
                activeCnt += active;
                freeCnt += free;
            }
            SkASSERT(activeCnt == block->fNodesInUse);
            activeNode = iter.next();
        }
        SkASSERT(count == fCount);
#endif
    }

    NodeList fList;
    NodeList fFreeList;
    int fCount;
    int fAllocCnt;
};