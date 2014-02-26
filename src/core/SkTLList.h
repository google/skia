/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTLList_DEFINED
#define SkTLList_DEFINED

#include "SkTInternalLList.h"
#include "SkTemplates.h"

template <typename T> class SkTLList;
template <typename T>
inline void* operator new(size_t, SkTLList<T>* list,
                          typename SkTLList<T>::Placement placement,
                          const typename SkTLList<T>::Iter& location);

/** Doubly-linked list of objects. The objects' lifetimes are controlled by the list. I.e. the
    the list creates the objects and they are deleted upon removal. This class block-allocates
    space for entries based on a param passed to the constructor.

    Elements of the list can be constructed in place using the following macros:
        SkNEW_INSERT_IN_LLIST_BEFORE(list, location, type_name, args)
        SkNEW_INSERT_IN_LLIST_AFTER(list, location, type_name, args)
    where list is a SkTLList<type_name>*, location is an iterator, and args is the paren-surrounded
    constructor arguments for type_name. These macros behave like addBefore() and addAfter().
*/
template <typename T>
class SkTLList : public SkNoncopyable {
private:
    struct Block;
    struct Node {
        char fObj[sizeof(T)];
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Node);
        Block* fBlock; // owning block.
    };
    typedef SkTInternalLList<Node> NodeList;

public:

    class Iter;

    /** allocCnt is the number of objects to allocate as a group. In the worst case fragmentation
        each object is using the space required for allocCnt unfragmented objects. */
    SkTLList(int allocCnt = 1) : fCount(0), fAllocCnt(allocCnt) {
        SkASSERT(allocCnt > 0);
        this->validate();
    }

    ~SkTLList() {
        this->validate();
        typename NodeList::Iter iter;
        Node* node = iter.init(fList, Iter::kHead_IterStart);
        while (NULL != node) {
            SkTCast<T*>(node->fObj)->~T();
            Block* block = node->fBlock;
            node = iter.next();
            if (0 == --block->fNodesInUse) {
                for (int i = 0; i < fAllocCnt; ++i) {
                    block->fNodes[i].~Node();
                }
                sk_free(block);
            }
        }
    }

    T* addToHead(const T& t) {
        this->validate();
        Node* node = this->createNode();
        fList.addToHead(node);
        SkNEW_PLACEMENT_ARGS(node->fObj, T, (t));
        this->validate();
        return reinterpret_cast<T*>(node->fObj);
    }

    T* addToHead() {
        this->validate();
        Node* node = this->createNode();
        fList.addToHead(node);
        SkNEW_PLACEMENT(node->fObj, T);
        this->validate();
        return reinterpret_cast<T*>(node->fObj);
    }

    T* addToTail(const T& t) {
        this->validate();
        Node* node = this->createNode();
        fList.addToTail(node);
        SkNEW_PLACEMENT_ARGS(node->fObj, T, (t));
        this->validate();
        return reinterpret_cast<T*>(node->fObj);
    }

    T* addToTail() {
        this->validate();
        Node* node = this->createNode();
        fList.addToTail(node);
        SkNEW_PLACEMENT(node->fObj, T);
        this->validate();
        return reinterpret_cast<T*>(node->fObj);
    }

    /** Adds a new element to the list before the location indicated by the iterator. If the
        iterator refers to a NULL location then the new element is added at the tail */
    T* addBefore(const T& t, const Iter& location) {
        return SkNEW_PLACEMENT_ARGS(this->internalAddBefore(location), T, (t));
    }

    /** Adds a new element to the list after the location indicated by the iterator. If the
        iterator refers to a NULL location then the new element is added at the head */
    T* addAfter(const T& t, const Iter& location) {
        return SkNEW_PLACEMENT_ARGS(this->internalAddAfter(location), T, (t));
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
        SkASSERT(reinterpret_cast<T*>(node->fObj) == t);
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
    bool isEmpty() const { this->validate(); return 0 == fCount; }

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
            SkASSERT(NULL != b.get()); // already checked that counts match.
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
            if (NULL != node) {
                return reinterpret_cast<T*>(node->fObj);
            } else {
                return NULL;
            }
        }
    };

    // For use with operator new
    enum Placement {
        kBefore_Placement,
        kAfter_Placement,
    };

private:
    struct Block {
        int fNodesInUse;
        Node fNodes[1];
    };

    size_t blockSize() const { return sizeof(Block) + sizeof(Node) * (fAllocCnt-1); }

    Node* createNode() {
        Node* node = fFreeList.head();
        if (NULL != node) {
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
        SkTCast<T*>(node->fObj)->~T();
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
        typename NodeList::Iter iter;
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

    // Support in-place initializing of objects inserted into the list via operator new.
    friend void* operator new<T>(size_t,
                                 SkTLList* list,
                                 Placement placement,
                                 const Iter& location);


    // Helpers that insert the node and returns a pointer to where the new object should be init'ed.
    void* internalAddBefore(Iter location) {
        this->validate();
        Node* node = this->createNode();
        fList.addBefore(node, location.getNode());
        this->validate();
        return node->fObj;
    }

    void* internalAddAfter(Iter location) {
        this->validate();
        Node* node = this->createNode();
        fList.addAfter(node, location.getNode());
        this->validate();
        return node->fObj;
    }

    NodeList fList;
    NodeList fFreeList;
    int fCount;
    int fAllocCnt;

};

// Use the below macros rather than calling this directly
template <typename T>
void *operator new(size_t, SkTLList<T>* list,
                   typename SkTLList<T>::Placement placement,
                   const typename SkTLList<T>::Iter& location) {
    SkASSERT(NULL != list);
    if (SkTLList<T>::kBefore_Placement == placement) {
        return list->internalAddBefore(location);
    } else {
        return list->internalAddAfter(location);
    }
}

// Skia doesn't use C++ exceptions but it may be compiled with them enabled. Having an op delete
// to match the op new silences warnings about missing op delete when a constructor throws an
// exception.
template <typename T>
void operator delete(void*,
                     SkTLList<T>*,
                     typename SkTLList<T>::Placement,
                     const typename SkTLList<T>::Iter&) {
    SK_CRASH();
}

#define SkNEW_INSERT_IN_LLIST_BEFORE(list, location, type_name, args) \
    (new ((list), SkTLList< type_name >::kBefore_Placement, (location)) type_name args)

#define SkNEW_INSERT_IN_LLIST_AFTER(list, location, type_name, args) \
    (new ((list), SkTLList< type_name >::kAfter_Placement, (location)) type_name args)

#define SkNEW_INSERT_AT_LLIST_HEAD(list, type_name, args) \
    SkNEW_INSERT_IN_LLIST_BEFORE((list), (list)->headIter(), type_name, args)

#define SkNEW_INSERT_AT_LLIST_TAIL(list, type_name, args) \
    SkNEW_INSERT_IN_LLIST_AFTER((list), (list)->tailIter(), type_name, args)

#endif
