/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGNode.h"

namespace sksg {

enum Flags {
    kMultiParents_Flag = 1 << 0,
    kInTraversal_Flag  = 1 << 1,
};

class Node::ScopedFlag {
public:
    ScopedFlag(Node* node, uint32_t flag)
        : fNode(node)
        , fOrigFlags(node->fFlags) {
        node->fFlags |= flag;
    }
    ~ScopedFlag() {
        fNode->fFlags = fOrigFlags;
    }

private:
    Node*    fNode;
    uint32_t fOrigFlags;
};

#define TRAVERSAL_GUARD                     \
    if (this->fFlags & kInTraversal_Flag) { \
        return;                             \
    }                                       \
    ScopedFlag traversal_guard(this, kInTraversal_Flag);

Node::Node()
    : fParent(nullptr)
    , fFlags(0)
    , fInvalState(kFull_Inval) {}

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

void Node::invalidate(uint32_t invalMask) {
    TRAVERSAL_GUARD

    if (invalMask == (fInvalState & invalMask)) {
        return;
    }

    fInvalState |= invalMask;
    forEachParent([&](Node* parent) {
        parent->invalidate(invalMask);
    });
}

} // namespace sksg
