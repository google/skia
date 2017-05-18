/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTree.h"

SkRTree::SkRTree(SkScalar aspectRatio)
    : fCount(0), fAspectRatio(isfinite(aspectRatio) ? aspectRatio : 1) {}

SkRect SkRTree::getRootBound() const {
    if (fCount) {
        return fRoot.fBounds;
    } else {
        return SkRect::MakeEmpty();
    }
}

void SkRTree::insert(const SkRect boundsArray[], int N) {
    SkASSERT(0 == fCount);

    SkTDArray<Branch> branches;
    branches.setReserve(N);

    for (int i = 0; i < N; i++) {
        const SkRect& bounds = boundsArray[i];
        if (bounds.isEmpty()) {
            continue;
        }

        Branch* b = branches.push();
        b->fBounds = bounds;
        b->fOpIndex = i;
    }

    fCount = branches.count();
    if (fCount) {
        if (1 == fCount) {
            fNodes.setReserve(1);
            Node* n = this->allocateNodeAtLevel(0);
            n->fNumChildren = 1;
            n->fChildren[0] = branches[0];
            fRoot.fSubtree = n;
            fRoot.fBounds  = branches[0].fBounds;
        } else {
            fNodes.setReserve(CountNodes(fCount, fAspectRatio));
            fRoot = this->bulkLoad(&branches);
        }
    }
}

SkRTree::Node* SkRTree::allocateNodeAtLevel(uint16_t level) {
    SkDEBUGCODE(Node* p = fNodes.begin());
    Node* out = fNodes.push();
    SkASSERT(fNodes.begin() == p);  // If this fails, we didn't setReserve() enough.
    out->fNumChildren = 0;
    out->fLevel = level;
    return out;
}

// This function parallels bulkLoad, but just counts how many nodes bulkLoad would allocate.
int SkRTree::CountNodes(int branches, SkScalar aspectRatio) {
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
    int numStrips = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(numBranches) / aspectRatio));
    int numTiles  = SkScalarCeilToInt(SkIntToScalar(numBranches) / SkIntToScalar(numStrips));
    int currentBranch = 0;
    int nodes = 0;
    for (int i = 0; i < numStrips; ++i) {
        for (int j = 0; j < numTiles && currentBranch < branches; ++j) {
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
    }
    return nodes + CountNodes(nodes, aspectRatio);
}

SkRTree::Branch SkRTree::bulkLoad(SkTDArray<Branch>* branches, int level) {
    if (branches->count() == 1) { // Only one branch.  It will be the root.
        return (*branches)[0];
    }

    // We might sort our branches here, but we expect Blink gives us a reasonable x,y order.
    // Skipping a call to sort (in Y) here resulted in a 17% win for recording with negligible
    // difference in playback speed.
    int numBranches = branches->count() / kMaxChildren;
    int remainder   = branches->count() % kMaxChildren;
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

    int numStrips = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(numBranches) / fAspectRatio));
    int numTiles  = SkScalarCeilToInt(SkIntToScalar(numBranches) / SkIntToScalar(numStrips));
    int currentBranch = 0;

    for (int i = 0; i < numStrips; ++i) {
        // Might be worth sorting by X here too.
        for (int j = 0; j < numTiles && currentBranch < branches->count(); ++j) {
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
            for (int k = 1; k < incrementBy && currentBranch < branches->count(); ++k) {
                b.fBounds.join((*branches)[currentBranch].fBounds);
                n->fChildren[k] = (*branches)[currentBranch];
                ++n->fNumChildren;
                ++currentBranch;
            }
            (*branches)[newBranches] = b;
            ++newBranches;
        }
    }
    branches->setCount(newBranches);
    return this->bulkLoad(branches, level + 1);
}

void SkRTree::search(const SkRect& query, SkTDArray<int>* results) const {
    if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
        this->search(fRoot.fSubtree, query, results);
    }
}

void SkRTree::search(Node* node, const SkRect& query, SkTDArray<int>* results) const {
    for (int i = 0; i < node->fNumChildren; ++i) {
        if (SkRect::Intersects(node->fChildren[i].fBounds, query)) {
            if (0 == node->fLevel) {
                results->push(node->fChildren[i].fOpIndex);
            } else {
                this->search(node->fChildren[i].fSubtree, query, results);
            }
        }
    }
}

size_t SkRTree::bytesUsed() const {
    size_t byteCount = sizeof(SkRTree);

    byteCount += fNodes.reserved() * sizeof(Node);

    return byteCount;
}
