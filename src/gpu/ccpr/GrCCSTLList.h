/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCSTLList_DEFINED
#define GrCCSTLList_DEFINED

#include "SkArenaAlloc.h"

/**
 * A singly-linked list whose head element is a local class member. This is required by
 * GrCCDrawPathsOp because the owning opList is unknown at the time of creation, so we can't use its
 * associated allocator to create the first element.
 */
template<typename T> class GrCCSTLList {
public:
    GrCCSTLList(T&& head) : fHead(std::move(head)) {}

    ~GrCCSTLList() {
        T* draw = fHead.fNext; // fHead will be destructed automatically.
        while (draw) {
            T* next = draw->fNext;
            draw->~T();
            draw = next;
        }
    }

    const T& head() const { return fHead; }
    T& head() { return fHead; }

    void append(GrCCSTLList&& right, SkArenaAlloc* alloc) {
        T* nextTail = (&right.fHead == right.fTail) ? nullptr : right.fTail;
        T* newRightHead =
                new (alloc->makeBytesAlignedTo(sizeof(T), alignof(T))) T(std::move(right.fHead));

        // Finish the move of right.fHead.
        right.fHead.fNext = nullptr;
        right.fTail = &right.fHead;

        fTail->fNext = newRightHead;
        fTail = !nextTail ? newRightHead : nextTail;
    }

    struct Iter {
        bool operator!=(const Iter& that) { return fCurr != that.fCurr; }
        const T& operator*() { return *fCurr; }
        void operator++() { fCurr = fCurr->fNext; }
        const T* fCurr;
    };
    Iter begin() const { return Iter{&fHead}; }
    Iter end() const { return Iter{nullptr}; }

private:
    T fHead;
    T* fTail = &fHead;
};

#endif
