/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/sksg/include/SkSGNode.h"
#include "src/core/SkRectPriv.h"

#include <algorithm>

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
            fNode->fFlags &= ~fFlag;
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
    , fFlags(kInvalidated_Flag)
    , fNodeFlags(0) {}

Node::~Node() {
    if (fFlags & kObserverArray_Flag) {
        SkASSERT(fInvalObserverArray->empty());
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

        auto observers = new std::vector<Node*>();
        observers->reserve(2);
        observers->push_back(node->fInvalObserver);

        node->fInvalObserverArray = observers;
        node->fFlags |= kObserverArray_Flag;
    }

    // No duplicate observers.
    SkASSERT(std::find(node->fInvalObserverArray->begin(),
                       node->fInvalObserverArray->end(), this) == node->fInvalObserverArray->end());

    node->fInvalObserverArray->push_back(this);
}

void Node::unobserveInval(const sk_sp<Node>& node) {
    SkASSERT(node);
    if (!(node->fFlags & kObserverArray_Flag)) {
        SkASSERT(node->fInvalObserver == this);
        node->fInvalObserver = nullptr;
        return;
    }

    SkDEBUGCODE(const auto origSize = node->fInvalObserverArray->size());
    node->fInvalObserverArray->erase(std::remove(node->fInvalObserverArray->begin(),
                                                 node->fInvalObserverArray->end(), this),
                                     node->fInvalObserverArray->end());
    SkASSERT(node->fInvalObserverArray->size() == origSize - 1);
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

    const auto generate_damage =
            ic && ((fFlags & kDamage_Flag) || (fInvalTraits & kOverrideDamage_Trait));
    if (!generate_damage) {
        // Trivial transitive revalidation.
        fBounds = this->onRevalidate(ic, ctm);
    } else {
        // Revalidate and emit damage for old-bounds, new-bounds.
        const auto prev_bounds = fBounds;

        auto* ic_override = (fInvalTraits & kOverrideDamage_Trait) ? nullptr : ic;
        fBounds = this->onRevalidate(ic_override, ctm);

        ic->inval(prev_bounds, ctm);
        if (fBounds != prev_bounds) {
            ic->inval(fBounds, ctm);
        }
    }

    fFlags &= ~(kInvalidated_Flag | kDamage_Flag);

    return fBounds;
}

} // namespace sksg
