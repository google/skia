/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTArenaList_DEFINED
#define GrTArenaList_DEFINED

#include "src/core/SkArenaAlloc.h"

// A singly-linked list whose nodes are allocated in an SkArenaAlloc.
template<typename T> class GrTArenaList {
public:
    struct Node {
        template <typename... Args>
        Node(Args&&... elementArgs) : fElement(std::forward<Args>(elementArgs)...) {}
        T fElement;
        Node* fNext = nullptr;
    };

    GrTArenaList() = default;

    template <typename... Args>
    GrTArenaList(SkArenaAlloc* allocator, Args&&... headArgs)
            : fHead(allocator->make<Node>(std::forward<Args>(headArgs)...)), fTail(fHead) {}

    void concat(GrTArenaList&& list) {
        if (!fHead) {
            *this = list;
            return;
        }
        SkASSERT(fTail);
        fTail->fNext = list.fHead;
        fTail = list.fTail;
    }

    struct Iter {
        bool operator!=(const Iter& it) const { return fCurr != it.fCurr; }
        bool operator==(const Iter& it) const { return fCurr == it.fCurr; }
        void operator++() { fCurr = fCurr->fNext; }
        const T& operator*() { return fCurr->fElement; }
        const Node* fCurr;
    };

    Iter begin() const { return Iter{fHead}; }
    Iter end() const { return Iter{nullptr}; }

private:
    Node* fHead = nullptr;
    Node* fTail = nullptr;
};

#endif
