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
    : fParent(nullptr)
    , fFlags(kInvalidated_Flag) {}

Node::~Node() {
    if (fFlags & kMultiParents_Flag) {
        SkASSERT(fParentArray->isEmpty());
        delete fParentArray;
    } else {
        SkASSERT(!fParent);
    }
}

void Node::addParent(Node* parent) {
    if (!(fFlags & kMultiParents_Flag)) {
        if (!fParent) {
            fParent = parent;
            return;
        }

        auto parents = new SkTDArray<Node*>();
        parents->setReserve(2);
        parents->push(fParent);

        fParentArray = parents;
        fFlags |= kMultiParents_Flag;
    }

    // should we allow duplicate refs?
    SkASSERT(fParentArray->find(parent) < 0);

    fParentArray->push(parent);
}

void Node::removeParent(Node* parent) {
    if (!(fFlags & kMultiParents_Flag)) {
        SkASSERT(fParent == parent);
        fParent= nullptr;
        return;
    }

    const auto idx = fParentArray->find(parent);
    SkASSERT(idx >= 0);
    fParentArray->remove(idx);
}

template <typename Func>
void Node::forEachParent(Func&& func) {
    if (fFlags & kMultiParents_Flag) {
        for (const auto& parent : *fParentArray) {
            func(parent);
        }
        return;
    }

    if (fParent) {
        func(fParent);
    }
}

void Node::invalidate() {
    TRAVERSAL_GUARD

    if (this->isInvalidated()) {
        return;
    }

    fFlags |= kInvalidated_Flag;
    forEachParent([&](Node* parent) {
        parent->invalidate();
    });
}

void Node::revalidate(InvalidationController* ic) {
    TRAVERSAL_GUARD

    if (this->isInvalidated()) {
        this->onRevalidate(ic);
        fFlags &= ~kInvalidated_Flag;
    }
}

} // namespace sksg
