/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGNode.h"

namespace sksg {

class Node::ScopedFlag {
public:
    ScopedFlag(Node* node, uint32_t flag)
        : fNode(node)
        , fFlag(flag) {
        SkASSERT(!(fNode->fFlags & fFlag));
        fNode->fFlags |= fFlag;
    }
    ~ScopedFlag() {
        fNode->fFlags &= ~fFlag;;
    }

private:
    Node*    fNode;
    uint32_t fFlag;
};

#define TRAVERSAL_GUARD                     \
    if (this->fFlags & kInTraversal_Flag) { \
        return;                             \
    }                                       \
    ScopedFlag traversal_guard(this, kInTraversal_Flag);

Node::Node()
    : fInvalReceiver(nullptr)
    , fFlags(kInvalidated_Flag) {}

Node::~Node() {
    if (fFlags & kReceiverArray_Flag) {
        SkASSERT(fInvalReceiverArray->isEmpty());
        delete fInvalReceiverArray;
    } else {
        SkASSERT(!fInvalReceiver);
    }
}

void Node::addInvalReceiver(Node* receiver) {
    if (!(fFlags & kReceiverArray_Flag)) {
        if (!fInvalReceiver) {
            fInvalReceiver = receiver;
            return;
        }

        auto receivers = new SkTDArray<Node*>();
        receivers->setReserve(2);
        receivers->push(fInvalReceiver);

        fInvalReceiverArray = receivers;
        fFlags |= kReceiverArray_Flag;
    }

    // No duplicate receivers.
    SkASSERT(fInvalReceiverArray->find(receiver) < 0);

    fInvalReceiverArray->push(receiver);
}

void Node::removeInvalReceiver(Node* receiver) {
    if (!(fFlags & kReceiverArray_Flag)) {
        SkASSERT(fInvalReceiver == receiver);
        fInvalReceiver = nullptr;
        return;
    }

    const auto idx = fInvalReceiverArray->find(receiver);
    SkASSERT(idx >= 0);
    fInvalReceiverArray->remove(idx);
}

template <typename Func>
void Node::forEachInvalReceiver(Func&& func) const {
    if (fFlags & kReceiverArray_Flag) {
        for (const auto& parent : *fInvalReceiverArray) {
            func(parent);
        }
        return;
    }

    if (fInvalReceiver) {
        func(fInvalReceiver);
    }
}

void Node::invalidate() {
    TRAVERSAL_GUARD

    if (this->isInvalidated()) {
        return;
    }

    fFlags |= kInvalidated_Flag;
    forEachInvalReceiver([&](Node* receiver) {
        receiver->invalidate();
    });
}

void Node::revalidate(InvalidationController* ic, const SkMatrix& ctm) {
    TRAVERSAL_GUARD

    if (this->isInvalidated()) {
        this->onRevalidate(ic, ctm);
        fFlags &= ~kInvalidated_Flag;
    }
}

} // namespace sksg
