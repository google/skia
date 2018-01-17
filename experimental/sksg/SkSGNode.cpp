/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRectPriv.h"
#include "SkSGNode.h"
#include "SkSGInvalidationController.h"

namespace sksg {

class Node::ScopedFlag {
public:
    ScopedFlag(Node* node, uint32_t flag)
        : fNode(node)
        , fFlag(flag)
        , fWasSet(node->fFlags & flag) {
        node->fFlags |= flag;
    }
    ~ScopedFlag() {
        if (!fWasSet) {
            fNode->fFlags &= ~fFlag;;
        }
    }

    bool wasSet() const { return fWasSet; }

private:
    Node*    fNode;
    uint32_t fFlag;
    bool     fWasSet;
};

#define TRAVERSAL_GUARD                                  \
    ScopedFlag traversal_guard(this, kInTraversal_Flag); \
    if (traversal_guard.wasSet())                        \
        return

Node::Node(uint32_t invalTraits)
    : fInvalReceiver(nullptr)
    , fBounds(SkRectPriv::MakeLargeS32())
    , fInvalTraits(invalTraits)
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

void Node::invalidate(bool damageBubbling) {
    TRAVERSAL_GUARD;

    if (this->hasInval() && (!damageBubbling || (fFlags & kDamage_Flag))) {
        // All done.
        return;
    }

    if (damageBubbling && !(fInvalTraits & kBubbleDamage_Trait)) {
        // Found a damage receiver.
        fFlags |= kDamage_Flag;
        damageBubbling = false;
    }

    fFlags |= kInvalidated_Flag;

    forEachInvalReceiver([&](Node* receiver) {
        receiver->invalidate(damageBubbling);
    });
}

const SkRect& Node::revalidate(InvalidationController* ic, const SkMatrix& ctm) {
    TRAVERSAL_GUARD fBounds;

    if (!this->hasInval()) {
        return fBounds;
    }

    SkRect prevBounds;
    if (fFlags & kDamage_Flag) {
        prevBounds = fBounds;
    }

    fBounds = this->onRevalidate(ic, ctm);

    if (fFlags & kDamage_Flag) {
        ic->inval(prevBounds, ctm);
        if (fBounds != prevBounds) {
            ic->inval(fBounds, ctm);
        }
    }

    fFlags &= ~(kInvalidated_Flag | kDamage_Flag);

    return fBounds;
}

} // namespace sksg
