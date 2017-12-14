/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGNode.h"

namespace sksg {

enum Flags {
    kMultiRefs_Flag = 1 << 0,
};

Node::Node()
    : fRef(nullptr)
    , fFlags(0) {}

Node::~Node() {
    if (fFlags & kMultiRefs_Flag) {
        SkASSERT(fRefArray->isEmpty());
        delete fRefArray;
    } else {
        SkASSERT(!fRef);
    }
}

void Node::addRef(Node* ref) {
    if (!(fFlags & kMultiRefs_Flag)) {
        if (!fRef) {
            fRef = ref;
            return;
        }

        auto refArray = new SkTDArray<Node*>();
        refArray->setReserve(2);
        refArray->push(fRef);

        fRefArray = refArray;
        fFlags |= kMultiRefs_Flag;
    }

    // should we allow duplicate refs?
    SkASSERT(fRefArray->find(ref) < 0);

    fRefArray->push(ref);
}

void Node::removeRef(Node* ref) {
    if (!(fFlags & kMultiRefs_Flag)) {
        fRef = nullptr;
        return;
    }

    const auto idx = fRefArray->find(ref);
    SkASSERT(idx >= 0);
    fRefArray->remove(idx);
}

} // namespace sksg
