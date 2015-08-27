/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDPQueue_DEFINED
#define SkTDPQueue_DEFINED

#include "SkTDArray.h"

/**
 * This class implements a priority queue. T is the type of the elements in the queue. LESS is a
 * function that compares two Ts and returns true if the first is higher priority than the second.
 *
 * Optionally objects may know their index into the priority queue. The queue will update the index
 * as the objects move through the queue. This is enabled by using a non-nullptr function for INDEX.
 * When an INDEX function is provided random deletes from the queue are allowed using remove().
 * Additionally, the * priority is allowed to change as long as priorityDidChange() is called
 * afterwards. In debug builds the index will be set to -1 before an element is removed from the
 * queue.
 */
template <typename T,
          bool (*LESS)(const T&, const T&),
          int* (*INDEX)(const T&) = (int* (*)(const T&))nullptr>
class SkTDPQueue : public SkNoncopyable {
public:
    SkTDPQueue() {}

    /** Number of items in the queue. */
    int count() const { return fArray.count(); }

    /** Gets the next item in the queue without popping it. */
    const T& peek() const { return fArray[0]; }
    T& peek() { return fArray[0]; }
    
    /** Removes the next item. */
    void pop() {
        this->validate();
        SkDEBUGCODE(if (SkToBool(INDEX)) { *INDEX(fArray[0]) = -1; })
        if (1 == fArray.count()) {
            fArray.pop();
            return;
        }

        fArray[0] = fArray[fArray.count() - 1];
        this->setIndex(0);
        fArray.pop();
        this->percolateDownIfNecessary(0);

        this->validate();
    }

    /** Inserts a new item in the queue based on its priority. */
    void insert(T entry) {
        this->validate();
        int index = fArray.count();
        *fArray.append() = entry;
        this->setIndex(fArray.count() - 1);
        this->percolateUpIfNecessary(index);
        this->validate();
    }

    /** Random access removal. This requires that the INDEX function is non-nullptr. */
    void remove(T entry) {
        SkASSERT(nullptr != INDEX);
        int index = *INDEX(entry);
        SkASSERT(index >= 0 && index < fArray.count());
        this->validate();
        SkDEBUGCODE(*INDEX(fArray[index]) = -1;)
        if (index == fArray.count() - 1) {
            fArray.pop();
            return;
        }
        fArray[index] = fArray[fArray.count() - 1];
        fArray.pop();
        this->setIndex(index);
        this->percolateUpOrDown(index);
        this->validate();
    }

    /** Notification that the priority of an entry has changed. This must be called after an
        item's priority is changed to maintain correct ordering. Changing the priority is only
        allowed if an INDEX function is provided. */
    void priorityDidChange(T entry) {
        SkASSERT(nullptr != INDEX);
        int index = *INDEX(entry);
        SkASSERT(index >= 0 && index < fArray.count());
        this->validate(index);
        this->percolateUpOrDown(index);
        this->validate();
    }

    /** Gets the item at index i in the priority queue (for i < this->count()). at(0) is equivalent
        to peek(). Otherwise, there is no guarantee about ordering of elements in the queue. */
    T at(int i) const { return fArray[i]; }

private:
    static int LeftOf(int x) { SkASSERT(x >= 0); return 2 * x + 1; }
    static int ParentOf(int x) { SkASSERT(x > 0); return (x - 1) >> 1; }

    void percolateUpOrDown(int index) {
        SkASSERT(index >= 0);
        if (!percolateUpIfNecessary(index)) {
            this->validate(index);
            this->percolateDownIfNecessary(index);
        }
    }

    bool percolateUpIfNecessary(int index) {
        SkASSERT(index >= 0);
        bool percolated = false;
        do {
            if (0 == index) {
                this->setIndex(index);
                return percolated;
            }
            int p = ParentOf(index);
            if (LESS(fArray[index], fArray[p])) {
                SkTSwap(fArray[index], fArray[p]);
                this->setIndex(index);
                index = p;
                percolated = true;
            } else {
                this->setIndex(index);
                return percolated;
            }
            this->validate(index);
        } while (true);
    }

    void percolateDownIfNecessary(int index) {
        SkASSERT(index >= 0);
        do {
            int child = LeftOf(index);
            
            if (child >= fArray.count()) {
                // We're a leaf.
                this->setIndex(index);
                return;
            }

            if (child + 1 >= fArray.count()) {
                // We only have a left child.
                if (LESS(fArray[child], fArray[index])) {
                    SkTSwap(fArray[child], fArray[index]);
                    this->setIndex(child);
                    this->setIndex(index);
                    return;
                }
            } else if (LESS(fArray[child + 1], fArray[child])) {
                // The right child is the one we should swap with, if we swap.
                child++;
            }

            // Check if we need to swap.
            if (LESS(fArray[child], fArray[index])) {
                SkTSwap(fArray[child], fArray[index]);
                this->setIndex(index);
                index = child;
            } else {
                // We're less than both our children.
                this->setIndex(index);
                return;
            }
            this->validate(index);
        } while (true);
    }

    void setIndex(int index) {
        SkASSERT(index < fArray.count());
        if (SkToBool(INDEX)) {
            *INDEX(fArray[index]) = index;
        }
    }

    void validate(int excludedIndex = -1) const {
#ifdef SK_DEBUG
        for (int i = 1; i < fArray.count(); ++i) {
            int p = ParentOf(i);
            if (excludedIndex != p && excludedIndex != i) {
                SkASSERT(!(LESS(fArray[i], fArray[p])));
                SkASSERT(!SkToBool(INDEX) || *INDEX(fArray[i]) == i);
            }
        }
#endif
    }

    SkTDArray<T> fArray;
    
    typedef SkNoncopyable INHERITED;
};

#endif
