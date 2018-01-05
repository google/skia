/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

Node::Node()
    : fInvalReceiver(nullptr)
    , fBounds(SkRect::MakeLargestS32())
    , fFlags(kInvalSelf_Flag | kInvalDescendant_Flag) {}

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

void Node::invalidateSelf() {
    if (this->hasSelfInval()) {
        return;
    }

    fFlags |= kInvalSelf_Flag;
    this->invalidateAncestors();
}

void Node::invalidateAncestors() {
    TRAVERSAL_GUARD;

    forEachInvalReceiver([&](Node* receiver) {
        if (receiver->hasDescendantInval()) {
            return;
        }
        receiver->fFlags |= kInvalDescendant_Flag;
        receiver->invalidateAncestors();
    });
}

const SkRect& Node::revalidate(InvalidationController* ic, const SkMatrix& ctm) {
    TRAVERSAL_GUARD fBounds;

    if (!this->hasInval()) {
        return fBounds;
    }

    const auto result     = this->onRevalidate(ic, ctm);
    const auto selfDamage = result.fDamage == Damage::kForceSelf ||
                            (this->hasSelfInval() && result.fDamage != Damage::kBlockSelf);

    if (selfDamage) {
        // old bounds
        ic->inval(fBounds, ctm);
        if (result.fBounds != fBounds) {
            // new bounds
            ic->inval(result.fBounds, ctm);
        }
    }

    fBounds = result.fBounds;
    fFlags &= ~(kInvalSelf_Flag | kInvalDescendant_Flag);

    return fBounds;
}

} // namespace sksg
