/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSinglyLinkedList_DEFINED
#define SkSinglyLinkedList_DEFINED

#include <utility>

#include "SkTypes.h"

template <typename T> class SkSinglyLinkedList {
    struct Node;
public:
    SkSinglyLinkedList() : fHead(nullptr), fTail(nullptr) {}
    ~SkSinglyLinkedList() { this->reset(); }
    void reset() {
        SkASSERT(fHead != nullptr || nullptr == fTail);
        // Use a while loop rather than recursion to avoid stack overflow.
        Node* node = fHead;
        while (node) {
            Node* next = node->fNext;
            SkASSERT(next != nullptr || node == fTail);
            delete node;
            node = next;
        }
        fHead = nullptr;
        fTail = nullptr;
    }
    T* back() { return fTail ? &fTail->fData : nullptr; }
    T* front() { return fHead ? &fHead->fData : nullptr; }
    bool empty() const { return fHead == nullptr; }
    #ifdef SK_DEBUG
    int count() {  // O(n), debug only.
        int count = 0;
        for (Node* node = fHead; node; node = node->fNext) {
            ++count;
        }
        return count;
    }
    #endif
    void pop_front() {
        if (Node* node = fHead) {
            fHead = node->fNext;
            delete node;
            if (fHead == nullptr) {
                fTail = nullptr;
            }
        }
    }
    template <class... Args> T* emplace_front(Args&&... args) {
        Node* n = new Node(std::forward<Args>(args)...);
        n->fNext = fHead;
        if (!fTail) {
            fTail = n;
            SkASSERT(!fHead);
        }
        fHead = n;
        return &n->fData;
    }
    template <class... Args> T* emplace_back(Args&&... args) {
        Node* n = new Node(std::forward<Args>(args)...);
        if (fTail) {
            fTail->fNext = n;
        } else {
            fHead = n;
        }
        fTail = n;
        return &n->fData;
    }
    class ConstIter {
    public:
        void operator++() { fNode = fNode->fNext; }
        const T& operator*() const { return fNode->fData; }
        bool operator!=(const ConstIter& rhs) const { return fNode != rhs.fNode; }
        ConstIter(const Node* n) : fNode(n) {}
    private:
        const Node* fNode;
    };
    ConstIter begin() const { return ConstIter(fHead); }
    ConstIter end() const { return ConstIter(nullptr); }

private:
    struct Node {
        T fData;
        Node* fNext;
        template <class... Args>
        Node(Args&&... args) : fData(std::forward<Args>(args)...), fNext(nullptr) {}
    };
    Node* fHead;
    Node* fTail;
    SkSinglyLinkedList(const SkSinglyLinkedList<T>&) = delete;
    SkSinglyLinkedList& operator=(const SkSinglyLinkedList<T>&) = delete;
};
#endif  // SkSinglyLinkedList_DEFINED
