/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSinglyLinkedList_DEFINED
#define SkSinglyLinkedList_DEFINED

#include <utility>

#include "SkMakeUnique.h"
#include "SkTypes.h"

template <typename T> class SkSinglyLinkedList {
    struct Node;
public:
    SkSinglyLinkedList() {}
    ~SkSinglyLinkedList() { this->reset(); }
    void reset() {
        SkASSERT(fHead != nullptr || nullptr == fTail);
        // Use a while loop rather than recursion to avoid stack overflow.
        std::unique_ptr<Node> node = std::move(fHead);
        while (node) {
            std::unique_ptr<Node> next = std::move(node->fNext);
            SkASSERT(next || node.get() == fTail);
            node = std::move(next);
        }
        fTail = nullptr;
    }
    T* back() { return fTail ? &fTail->fData : nullptr; }
    T* front() { return fHead ? &fHead->fData : nullptr; }
    bool empty() const { return fHead == nullptr; }
    #ifdef SK_DEBUG
    int count() {  // O(n), debug only.
        int count = 0;
        for (Node* node = fHead.get(); node; node = node->fNext.get()) {
            ++count;
        }
        return count;
    }
    #endif
    void pop_front() {
        if (fHead) {
            fHead = std::move(fHead->fNext);
            if (!fHead) {
                fTail = nullptr;
            }
        }
    }
    template <class... Args> T* emplace_front(Args&&... args) {
        fHead = skstd::make_unique<Node>(std::move(fHead), std::forward<Args>(args)...);
        if (!fTail) {
            fTail = fHead.get();
        }
        return &fHead->fData;
    }
    template <class... Args> T* emplace_back(Args&&... args) {
        std::unique_ptr<Node>* dst = fTail ? &fTail->fNext : &fHead;
        *dst = skstd::make_unique<Node>(nullptr, std::forward<Args>(args)...);
        fTail = dst->get();
        return &fTail->fData;
    }
    class ConstIter {
    public:
        void operator++() { fNode = fNode->fNext.get(); }
        const T& operator*() const { return fNode->fData; }
        bool operator!=(const ConstIter& rhs) const { return fNode != rhs.fNode; }
        ConstIter(const Node* n) : fNode(n) {}
    private:
        const Node* fNode;
    };
    ConstIter begin() const { return ConstIter(fHead.get()); }
    ConstIter end() const { return ConstIter(nullptr); }

private:
    struct Node {
        T fData;
        std::unique_ptr<Node> fNext;
        template <class... Args>
        Node(std::unique_ptr<Node> n, Args&&... args)
            : fData(std::forward<Args>(args)...), fNext(std::move(n)) {}
    };
    std::unique_ptr<Node> fHead;
    Node* fTail = nullptr;
    SkSinglyLinkedList(const SkSinglyLinkedList<T>&) = delete;
    SkSinglyLinkedList& operator=(const SkSinglyLinkedList<T>&) = delete;
};
#endif  // SkSinglyLinkedList_DEFINED
