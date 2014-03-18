/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFreeList_DEFINED
#define SkFreeList_DEFINED

#include "SkTInternalSList.h"

/**
 * An implementation of a self growing pool of objects.
 * It maintains a pool of fully initialized objects. If an attempt is made to
 * acquire one, and there are none left, it makes some more.
 * It does not automatically reclaim them, they have to be given back to it.
 * Constructors will be called on objects allocated by the pool at allocation
 * time.
 * All allocated objects will be destroyed and memory will be reclaimed when
 * the pool is destroyed, so the pool must survive longer than you are using
 * any item taken from it.
 */
template<typename T, int numItemsPerBlock = 4096/sizeof(T)> class SkTObjectPool {
public:
    SkTObjectPool() {}
    ~SkTObjectPool() {
        while (!fBlocks.isEmpty()) {
            SkDELETE(fBlocks.pop());
        }
    }

    /**
     * Get an item from the pool.
     * If the pool has no free items, it will allocate and construct some more.
     * The returned item is only valid as long as the pool has not been
     * destroyed, at that point all memory allocated by grow will have been
     * reclaimed.
     * This method is *not* thread safe.
     */
    T* acquire() {
        if (fAvailable.isEmpty()) {
            grow();
        }
        return fAvailable.pop();
    }

    /**
     * Release an item into the pool.
     * The item does not have to have come from the pool, but if it did not
     * it must have a lifetime greater than the pool does.
     * This method is *not* thread safe.
     */
    void release(T* entry) {
        fAvailable.push(entry);
    }

    /**
     * Takes all the items from an SkTInternalSList and adds them back to this
     * pool. The other list will be left empty.
     */
    void releaseAll(SkTInternalSList<T>* other) {
        fAvailable.pushAll(other);
    }

    /**
     * Returns the number of items immediately available without having to
     * construct any new ones.
     */
    int available() const { return fAvailable.getCount(); }

    /**
     * Returns the number of blocks of items the pool has allocated so far.
     */
    int blocks() const { return fBlocks.getCount(); }

    /**
     * Returns the number of items allocated by the pool in total.
     */
    int allocated() const { return fBlocks.getCount() * numItemsPerBlock; }

private:
    /**
     * The type for a new block of entries for the list.
     */
    struct Block {
        T entries[numItemsPerBlock];
        SK_DECLARE_INTERNAL_SLIST_INTERFACE(Block);
    };
    SkTInternalSList<Block> fBlocks;
    SkTInternalSList<T> fAvailable;

    /**
     * When the free list runs out of items, this method is called to allocate
     * a new block of them.
     * It calls the constructors and then pushes the nodes into the available
     * list.
     */
    void grow() {
        Block* block = SkNEW(Block);
        fBlocks.push(block);
        for(int index = 0; index < numItemsPerBlock; ++index) {
            fAvailable.push(&block->entries[index]);
        }
    }

};

#endif
