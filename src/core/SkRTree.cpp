/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRTree.h"

SkRTree::SkRTree() : fCount(0) {}

void SkRTree::insert(const SkRect boundsArray[], int N) {
    SkASSERT(0 == fCount);

    std::vector<Branch> branches;
    branches.reserve(N);

    for (int i = 0; i < N; i++) {
        const SkRect& bounds = boundsArray[i];
        if (bounds.isEmpty()) {
            continue;
        }

        Branch b;
        b.fBounds = bounds;
        b.fOpIndex = i;
        branches.push_back(b);
    }

    fCount = (int)branches.size();
    if (fCount) {
        if (1 == fCount) {
            fNodes.reserve(1);
            Node* n = this->allocateNodeAtLevel(0);
            n->fNumChildren = 1;
            n->fChildren[0] = branches[0];
            fRoot.fSubtree = n;
            fRoot.fBounds  = branches[0].fBounds;
        } else {
            fNodes.reserve(CountNodes(fCount));
            fRoot = this->bulkLoad(&branches);
        }
    }
}

SkRTree::Node* SkRTree::allocateNodeAtLevel(uint16_t level) {
    SkDEBUGCODE(Node* p = fNodes.data());
    fNodes.push_back(Node{});
    Node& out = fNodes.back();
    SkASSERT(fNodes.data() == p);  // If this fails, we didn't reserve() enough.
    out.fNumChildren = 0;
    out.fLevel = level;
    return &out;
}

// This function parallels bulkLoad, but just counts how many nodes bulkLoad would allocate.
int SkRTree::CountNodes(int branches) {
    if (branches == 1) {
        return 1;
    }
    int numBranches = branches / kMaxChildren;
    int remainder   = branches % kMaxChildren;
    if (remainder > 0) {
        numBranches++;
        if (remainder >= kMinChildren) {
            remainder = 0;
        } else {
            remainder = kMinChildren - remainder;
        }
    }
    int currentBranch = 0;
    int nodes = 0;
    while (currentBranch < branches) {
        int incrementBy = kMaxChildren;
        if (remainder != 0) {
            if (remainder <= kMaxChildren - kMinChildren) {
                incrementBy -= remainder;
                remainder = 0;
            } else {
                incrementBy = kMinChildren;
                remainder -= kMaxChildren - kMinChildren;
            }
        }
        nodes++;
        currentBranch++;
        for (int k = 1; k < incrementBy && currentBranch < branches; ++k) {
            currentBranch++;
        }
    }
    return nodes + CountNodes(nodes);
}

SkRTree::Branch SkRTree::bulkLoad(std::vector<Branch>* branches, int level) {
    if (branches->size() == 1) { // Only one branch.  It will be the root.
        return (*branches)[0];
    }

    // We might sort our branches here, but we expect Blink gives us a reasonable x,y order.
    // Skipping a call to sort (in Y) here resulted in a 17% win for recording with negligible
    // difference in playback speed.
    int numBranches = (int)branches->size() / kMaxChildren;
    int remainder   = (int)branches->size() % kMaxChildren;
    int newBranches = 0;

    if (remainder > 0) {
        ++numBranches;
        // If the remainder isn't enough to fill a node, we'll add fewer nodes to other branches.
        if (remainder >= kMinChildren) {
            remainder = 0;
        } else {
            remainder = kMinChildren - remainder;
        }
    }

    int currentBranch = 0;
    while (currentBranch < (int)branches->size()) {
        int incrementBy = kMaxChildren;
        if (remainder != 0) {
            // if need be, omit some nodes to make up for remainder
            if (remainder <= kMaxChildren - kMinChildren) {
                incrementBy -= remainder;
                remainder = 0;
            } else {
                incrementBy = kMinChildren;
                remainder -= kMaxChildren - kMinChildren;
            }
        }
        Node* n = allocateNodeAtLevel(level);
        n->fNumChildren = 1;
        n->fChildren[0] = (*branches)[currentBranch];
        Branch b;
        b.fBounds = (*branches)[currentBranch].fBounds;
        b.fSubtree = n;
        ++currentBranch;
        for (int k = 1; k < incrementBy && currentBranch < (int)branches->size(); ++k) {
            b.fBounds.join((*branches)[currentBranch].fBounds);
            n->fChildren[k] = (*branches)[currentBranch];
            ++n->fNumChildren;
            ++currentBranch;
        }
        (*branches)[newBranches] = b;
        ++newBranches;
    }
    branches->resize(newBranches);
    return this->bulkLoad(branches, level + 1);
}

void SkRTree::search(const SkRect& query, std::vector<int>* results) const {
    if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
        this->search(fRoot.fSubtree, query, results);
    }
}

void SkRTree::search(Node* node, const SkRect& query, std::vector<int>* results) const {
    for (int i = 0; i < node->fNumChildren; ++i) {
        if (SkRect::Intersects(node->fChildren[i].fBounds, query)) {
            if (0 == node->fLevel) {
                results->push_back(node->fChildren[i].fOpIndex);
            } else {
                this->search(node->fChildren[i].fSubtree, query, results);
            }
        }
    }
}

size_t SkRTree::bytesUsed() const {
    size_t byteCount = sizeof(SkRTree);

    byteCount += fNodes.capacity() * sizeof(Node);

    return byteCount;
}
