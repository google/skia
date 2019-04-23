/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArenaAllocList_DEFINED
#define SkArenaAllocList_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkArenaAlloc.h"

/**
 * A singly linked list of Ts stored in a SkArenaAlloc. The arena rather than the list owns
 * the elements. This supports forward iteration and range based for loops.
 */
template <typename T>
class SkArenaAllocList {
private:
    struct Node;

public:
    SkArenaAllocList() = default;

    void reset() { fHead = fTail = nullptr; }

    template <typename... Args>
    inline T& append(SkArenaAlloc* arena, Args... args);

    class Iter {
    public:
        Iter() = default;
        inline Iter& operator++();
        T& operator*() const { return fCurr->fT; }
        T* operator->() const { return &fCurr->fT; }
        bool operator==(const Iter& that) const { return fCurr == that.fCurr; }
        bool operator!=(const Iter& that) const { return !(*this == that); }

    private:
        friend class SkArenaAllocList;
        explicit Iter(Node* node) : fCurr(node) {}
        Node* fCurr = nullptr;
    };

    Iter begin() { return Iter(fHead); }
    Iter end() { return Iter(); }
    Iter tail() { return Iter(fTail); }

private:
    struct Node {
        template <typename... Args>
        Node(Args... args) : fT(std::forward<Args>(args)...) {}
        T fT;
        Node* fNext = nullptr;
    };
    Node* fHead = nullptr;
    Node* fTail = nullptr;
};

template <typename T>
template <typename... Args>
T& SkArenaAllocList<T>::append(SkArenaAlloc* arena, Args... args) {
    SkASSERT(!fHead == !fTail);
    auto* n = arena->make<Node>(std::forward<Args>(args)...);
    if (!fTail) {
        fHead = fTail = n;
    } else {
        fTail = fTail->fNext = n;
    }
    return fTail->fT;
}

template <typename T>
typename SkArenaAllocList<T>::Iter& SkArenaAllocList<T>::Iter::operator++() {
    fCurr = fCurr->fNext;
    return *this;
}

#endif
