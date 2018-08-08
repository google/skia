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
    : fInvalObserver(nullptr)
    , fBounds(SkRectPriv::MakeLargeS32())
    , fInvalTraits(invalTraits)
    , fFlags(kInvalidated_Flag) {}

Node::~Node() {
    if (fFlags & kObserverArray_Flag) {
        SkASSERT(fInvalObserverArray->isEmpty());
        delete fInvalObserverArray;
    } else {
        SkASSERT(!fInvalObserver);
    }
}

void Node::observeInval(const sk_sp<Node>& node) {
    SkASSERT(node);
    if (!(node->fFlags & kObserverArray_Flag)) {
        if (!node->fInvalObserver) {
            node->fInvalObserver = this;
            return;
        }

        auto observers = new SkTDArray<Node*>();
        observers->setReserve(2);
        observers->push(node->fInvalObserver);

        node->fInvalObserverArray = observers;
        node->fFlags |= kObserverArray_Flag;
    }

    // No duplicate observers.
    SkASSERT(node->fInvalObserverArray->find(this) < 0);

    node->fInvalObserverArray->push(this);
}

void Node::unobserveInval(const sk_sp<Node>& node) {
    SkASSERT(node);
    if (!(node->fFlags & kObserverArray_Flag)) {
        SkASSERT(node->fInvalObserver == this);
        node->fInvalObserver = nullptr;
        return;
    }

    const auto idx = node->fInvalObserverArray->find(this);
    SkASSERT(idx >= 0);
    node->fInvalObserverArray->remove(idx);
}

template <typename Func>
void Node::forEachInvalObserver(Func&& func) const {
    if (fFlags & kObserverArray_Flag) {
        for (const auto& parent : *fInvalObserverArray) {
            func(parent);
        }
        return;
    }

    if (fInvalObserver) {
        func(fInvalObserver);
    }
}

void Node::invalidate(bool damageBubbling) {
    TRAVERSAL_GUARD;

    if (this->hasInval() && (!damageBubbling || (fFlags & kDamage_Flag))) {
        // All done.
        return;
    }

    if (damageBubbling && !(fInvalTraits & kBubbleDamage_Trait)) {
        // Found a damage observer.
        fFlags |= kDamage_Flag;
        damageBubbling = false;
    }

    fFlags |= kInvalidated_Flag;

    forEachInvalObserver([&](Node* observer) {
        observer->invalidate(damageBubbling);
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
