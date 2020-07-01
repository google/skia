/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSTArenaList_DEFINED
#define GrSTArenaList_DEFINED

#include "src/core/SkArenaAlloc.h"

// A singly-linked list whose head element is a "stack allocated" class member and whose subsequent
// elements are allocated in an SkArenaAlloc.
template<typename T> class GrSTArenaList {
public:
    struct Node {
        template <typename... Args>
        Node(Args&&... elementArgs) : fElement(std::forward<Args>(elementArgs)...) {}
        T fElement;
        Node* fNext = nullptr;
    };

    template <typename... Args>
    GrSTArenaList(Args&&... headArgs) : fHead(std::forward<Args>(headArgs)...) {}

    const T& head() const { return fHead.fElement; }
    T& head() { return fHead.fElement; }

    void concat(GrSTArenaList&& list, SkArenaAlloc* allocator) {
        Node* listHeadCopy = allocator->make<Node>(std::move(list.fHead));
        fTail->fNext = listHeadCopy;
        // If the list's fTail pointed to its locally allocated head element, then point our fTail
        // at the copy we just made in the arena. Otherwise the list's fTail already points at an
        // arena-allocated element, so keep it.
        fTail = (list.fTail == &list.fHead) ? listHeadCopy : list.fTail;
    }

    struct Iter {
        bool operator!=(const Iter& it) const { return fCurr != it.fCurr; }
        bool operator==(const Iter& it) const { return fCurr == it.fCurr; }
        void operator++() { fCurr = fCurr->fNext; }
        T& operator*() { return fCurr->fElement; }
        Node* fCurr;
    };

    Iter begin() { return Iter{&fHead}; }
    Iter end() { return Iter{nullptr}; }

private:
    Node fHead;
    Node* fTail = &fHead;
};

#endif
