/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCSTLList_DEFINED
#define GrCCSTLList_DEFINED

#include "include/private/SkArenaAlloc.h"
#include "include/private/SkNoncopyable.h"
#include <new>

/**
 * A singly-linked list whose head element is a local class member. This is required by
 * GrCCDrawPathsOp because the owning opList is unknown at the time of creation, so we can't use its
 * associated allocator to create the first element.
 */
template<typename T> class GrCCSTLList : SkNoncopyable {
public:
    template <typename ...Args>
    GrCCSTLList(Args&&... args) : fHead(std::forward<Args>(args)...) {}

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

    template<typename U> struct Iter {
        bool operator!=(const Iter& that) { return fCurr != that.fCurr; }
        U& operator*() { return *fCurr; }
        void operator++() { fCurr = fCurr->fNext; }
        U* fCurr;
    };
    Iter<const T> begin() const { return Iter<const T>{&fHead}; }
    Iter<const T> end() const { return Iter<const T>{nullptr}; }
    Iter<T> begin() { return Iter<T>{&fHead}; }
    Iter<T> end() { return Iter<T>{nullptr}; }

private:
    T fHead;
    T* fTail = &fHead;
};

#endif
