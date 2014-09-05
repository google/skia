/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTree.h"
#include "SkTSort.h"

static inline uint32_t get_area(const SkIRect& rect);
static inline uint32_t get_overlap(const SkIRect& rect1, const SkIRect& rect2);
static inline uint32_t get_margin(const SkIRect& rect);
static inline uint32_t get_area_increase(const SkIRect& rect1, SkIRect rect2);
static inline void join_no_empty_check(const SkIRect& joinWith, SkIRect* out);

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRTree* SkRTree::Create(int minChildren, int maxChildren, SkScalar aspectRatio,
            bool sortWhenBulkLoading) {
    if (minChildren < maxChildren && (maxChildren + 1) / 2 >= minChildren &&
        minChildren > 0 && maxChildren < static_cast<int>(SK_MaxU16)) {
        return new SkRTree(minChildren, maxChildren, aspectRatio, sortWhenBulkLoading);
    }
    return NULL;
}

SkRTree::SkRTree(int minChildren, int maxChildren, SkScalar aspectRatio,
        bool sortWhenBulkLoading)
    : fMinChildren(minChildren)
    , fMaxChildren(maxChildren)
    , fNodeSize(sizeof(Node) + sizeof(Branch) * maxChildren)
    , fCount(0)
    , fNodes(fNodeSize * 256)
    , fAspectRatio(aspectRatio)
    , fSortWhenBulkLoading(sortWhenBulkLoading) {
    SkASSERT(minChildren < maxChildren && minChildren > 0 && maxChildren <
             static_cast<int>(SK_MaxU16));
    SkASSERT((maxChildren + 1) / 2 >= minChildren);
    this->validate();
}

SkRTree::~SkRTree() {
    this->clear();
}

void SkRTree::insert(void* data, const SkRect& fbounds, bool defer) {
    SkIRect bounds;
    if (fbounds.isLargest()) {
        bounds.setLargest();
    } else {
        fbounds.roundOut(&bounds);
    }

    this->validate();
    if (bounds.isEmpty()) {
        SkASSERT(false);
        return;
    }
    Branch newBranch;
    newBranch.fBounds = bounds;
    newBranch.fChild.data = data;
    if (this->isEmpty()) {
        // since a bulk-load into an existing tree is as of yet unimplemented (and arguably not
        // of vital importance right now), we only batch up inserts if the tree is empty.
        if (defer) {
            fDeferredInserts.push(newBranch);
            return;
        } else {
            fRoot.fChild.subtree = allocateNode(0);
            fRoot.fChild.subtree->fNumChildren = 0;
        }
    }

    Branch* newSibling = insert(fRoot.fChild.subtree, &newBranch);
    fRoot.fBounds = this->computeBounds(fRoot.fChild.subtree);

    if (newSibling) {
        Node* oldRoot = fRoot.fChild.subtree;
        Node* newRoot = this->allocateNode(oldRoot->fLevel + 1);
        newRoot->fNumChildren = 2;
        *newRoot->child(0) = fRoot;
        *newRoot->child(1) = *newSibling;
        fRoot.fChild.subtree = newRoot;
        fRoot.fBounds = this->computeBounds(fRoot.fChild.subtree);
    }

    ++fCount;
    this->validate();
}

void SkRTree::flushDeferredInserts() {
    this->validate();
    if (this->isEmpty() && fDeferredInserts.count() > 0) {
        fCount = fDeferredInserts.count();
        if (1 == fCount) {
            fRoot.fChild.subtree = allocateNode(0);
            fRoot.fChild.subtree->fNumChildren = 0;
            this->insert(fRoot.fChild.subtree, &fDeferredInserts[0]);
            fRoot.fBounds = fDeferredInserts[0].fBounds;
        } else {
            fRoot = this->bulkLoad(&fDeferredInserts);
        }
    } else {
        // TODO: some algorithm for bulk loading into an already populated tree
        SkASSERT(0 == fDeferredInserts.count());
    }
    fDeferredInserts.rewind();
    this->validate();
}

void SkRTree::search(const SkRect& fquery, SkTDArray<void*>* results) const {
    SkIRect query;
    fquery.roundOut(&query);
    this->validate();
    SkASSERT(0 == fDeferredInserts.count());  // If this fails, you should have flushed.
    if (!this->isEmpty() && SkIRect::IntersectsNoEmptyCheck(fRoot.fBounds, query)) {
        this->search(fRoot.fChild.subtree, query, results);
    }
    this->validate();
}

void SkRTree::clear() {
    this->validate();
    fNodes.reset();
    fDeferredInserts.rewind();
    fCount = 0;
    this->validate();
}

SkRTree::Node* SkRTree::allocateNode(uint16_t level) {
    Node* out = static_cast<Node*>(fNodes.allocThrow(fNodeSize));
    out->fNumChildren = 0;
    out->fLevel = level;
    return out;
}

SkRTree::Branch* SkRTree::insert(Node* root, Branch* branch, uint16_t level) {
    Branch* toInsert = branch;
    if (root->fLevel != level) {
        int childIndex = this->chooseSubtree(root, branch);
        toInsert = this->insert(root->child(childIndex)->fChild.subtree, branch, level);
        root->child(childIndex)->fBounds = this->computeBounds(
            root->child(childIndex)->fChild.subtree);
    }
    if (toInsert) {
        if (root->fNumChildren == fMaxChildren) {
            // handle overflow by splitting. TODO: opportunistic reinsertion

            // decide on a distribution to divide with
            Node* newSibling = this->allocateNode(root->fLevel);
            Branch* toDivide = SkNEW_ARRAY(Branch, fMaxChildren + 1);
            for (int i = 0; i < fMaxChildren; ++i) {
                toDivide[i] = *root->child(i);
            }
            toDivide[fMaxChildren] = *toInsert;
            int splitIndex = this->distributeChildren(toDivide);

            // divide up the branches
            root->fNumChildren = splitIndex;
            newSibling->fNumChildren = fMaxChildren + 1 - splitIndex;
            for (int i = 0; i < splitIndex; ++i) {
                *root->child(i) = toDivide[i];
            }
            for (int i = splitIndex; i < fMaxChildren + 1; ++i) {
                *newSibling->child(i - splitIndex) = toDivide[i];
            }
            SkDELETE_ARRAY(toDivide);

            // pass the new sibling branch up to the parent
            branch->fChild.subtree = newSibling;
            branch->fBounds = this->computeBounds(newSibling);
            return branch;
        } else {
            *root->child(root->fNumChildren) = *toInsert;
            ++root->fNumChildren;
            return NULL;
        }
    }
    return NULL;
}

int SkRTree::chooseSubtree(Node* root, Branch* branch) {
    SkASSERT(!root->isLeaf());
    if (1 < root->fLevel) {
        // root's child pointers do not point to leaves, so minimize area increase
        int32_t minAreaIncrease = SK_MaxS32;
        int32_t minArea         = SK_MaxS32;
        int32_t bestSubtree     = -1;
        for (int i = 0; i < root->fNumChildren; ++i) {
            const SkIRect& subtreeBounds = root->child(i)->fBounds;
            int32_t areaIncrease = get_area_increase(subtreeBounds, branch->fBounds);
            // break ties in favor of subtree with smallest area
            if (areaIncrease < minAreaIncrease || (areaIncrease == minAreaIncrease &&
                static_cast<int32_t>(get_area(subtreeBounds)) < minArea)) {
                minAreaIncrease = areaIncrease;
                minArea = get_area(subtreeBounds);
                bestSubtree = i;
            }
        }
        SkASSERT(-1 != bestSubtree);
        return bestSubtree;
    } else if (1 == root->fLevel) {
        // root's child pointers do point to leaves, so minimize overlap increase
        int32_t minOverlapIncrease = SK_MaxS32;
        int32_t minAreaIncrease    = SK_MaxS32;
        int32_t bestSubtree = -1;
        for (int32_t i = 0; i < root->fNumChildren; ++i) {
            const SkIRect& subtreeBounds = root->child(i)->fBounds;
            SkIRect expandedBounds = subtreeBounds;
            join_no_empty_check(branch->fBounds, &expandedBounds);
            int32_t overlap = 0;
            for (int32_t j = 0; j < root->fNumChildren; ++j) {
                if (j == i) continue;
                // Note: this would be more correct if we subtracted the original pre-expanded
                // overlap, but computing overlaps is expensive and omitting it doesn't seem to
                // hurt query performance. See get_overlap_increase()
                overlap += get_overlap(expandedBounds, root->child(j)->fBounds);
            }
            // break ties with lowest area increase
            if (overlap < minOverlapIncrease || (overlap == minOverlapIncrease &&
                static_cast<int32_t>(get_area_increase(branch->fBounds, subtreeBounds)) <
                minAreaIncrease)) {
                minOverlapIncrease = overlap;
                minAreaIncrease = get_area_increase(branch->fBounds, subtreeBounds);
                bestSubtree = i;
            }
        }
        return bestSubtree;
    } else {
        SkASSERT(false);
        return 0;
    }
}

SkIRect SkRTree::computeBounds(Node* n) {
    SkIRect r = n->child(0)->fBounds;
    for (int i = 1; i < n->fNumChildren; ++i) {
        join_no_empty_check(n->child(i)->fBounds, &r);
    }
    return r;
}

int SkRTree::distributeChildren(Branch* children) {
    // We have two sides to sort by on each of two axes:
    const static SortSide sorts[2][2] = {
        {&SkIRect::fLeft, &SkIRect::fRight},
        {&SkIRect::fTop, &SkIRect::fBottom}
    };

    // We want to choose an axis to split on, then a distribution along that axis; we'll need
    // three pieces of info: the split axis, the side to sort by on that axis, and the index
    // to split the sorted array on.
    int32_t sortSide = -1;
    int32_t k        = -1;
    int32_t axis     = -1;
    int32_t bestS    = SK_MaxS32;

    // Evaluate each axis, we want the min summed margin-value (s) over all distributions
    for (int i = 0; i < 2; ++i) {
        int32_t minOverlap   = SK_MaxS32;
        int32_t minArea      = SK_MaxS32;
        int32_t axisBestK    = 0;
        int32_t axisBestSide = 0;
        int32_t s = 0;

        // Evaluate each sort
        for (int j = 0; j < 2; ++j) {
            SkTQSort(children, children + fMaxChildren, RectLessThan(sorts[i][j]));

            // Evaluate each split index
            for (int32_t k = 1; k <= fMaxChildren - 2 * fMinChildren + 2; ++k) {
                SkIRect r1 = children[0].fBounds;
                SkIRect r2 = children[fMinChildren + k - 1].fBounds;
                for (int32_t l = 1; l < fMinChildren - 1 + k; ++l) {
                    join_no_empty_check(children[l].fBounds, &r1);
                }
                for (int32_t l = fMinChildren + k; l < fMaxChildren + 1; ++l) {
                    join_no_empty_check(children[l].fBounds, &r2);
                }

                int32_t area = get_area(r1) + get_area(r2);
                int32_t overlap = get_overlap(r1, r2);
                s += get_margin(r1) + get_margin(r2);

                if (overlap < minOverlap || (overlap == minOverlap && area < minArea)) {
                    minOverlap = overlap;
                    minArea = area;
                    axisBestSide = j;
                    axisBestK = k;
                }
            }
        }

        if (s < bestS) {
            bestS = s;
            axis = i;
            sortSide = axisBestSide;
            k = axisBestK;
        }
    }

    // replicate the sort of the winning distribution, (we can skip this if the last
    // sort ended up being best)
    if (!(axis == 1 && sortSide == 1)) {
        SkTQSort(children, children + fMaxChildren, RectLessThan(sorts[axis][sortSide]));
    }

    return fMinChildren - 1 + k;
}

void SkRTree::search(Node* root, const SkIRect query, SkTDArray<void*>* results) const {
    for (int i = 0; i < root->fNumChildren; ++i) {
        if (SkIRect::IntersectsNoEmptyCheck(root->child(i)->fBounds, query)) {
            if (root->isLeaf()) {
                results->push(root->child(i)->fChild.data);
            } else {
                this->search(root->child(i)->fChild.subtree, query, results);
            }
        }
    }
}

SkRTree::Branch SkRTree::bulkLoad(SkTDArray<Branch>* branches, int level) {
    if (branches->count() == 1) {
        // Only one branch: it will be the root
        Branch out = (*branches)[0];
        branches->rewind();
        return out;
    } else {
        // We sort the whole list by y coordinates, if we are told to do so.
        //
        // We expect Webkit / Blink to give us a reasonable x,y order.
        // Avoiding this call resulted in a 17% win for recording with
        // negligible difference in playback speed.
        if (fSortWhenBulkLoading) {
            SkTQSort(branches->begin(), branches->end() - 1, RectLessY());
        }

        int numBranches = branches->count() / fMaxChildren;
        int remainder = branches->count() % fMaxChildren;
        int newBranches = 0;

        if (0 != remainder) {
            ++numBranches;
            // If the remainder isn't enough to fill a node, we'll need to add fewer nodes to
            // some other branches to make up for it
            if (remainder >= fMinChildren) {
                remainder = 0;
            } else {
                remainder = fMinChildren - remainder;
            }
        }

        int numStrips = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(numBranches) *
                                     SkScalarInvert(fAspectRatio)));
        int numTiles = SkScalarCeilToInt(SkIntToScalar(numBranches) /
                                    SkIntToScalar(numStrips));
        int currentBranch = 0;

        for (int i = 0; i < numStrips; ++i) {
            // Once again, if we are told to do so, we sort by x.
            if (fSortWhenBulkLoading) {
                int begin = currentBranch;
                int end = currentBranch + numTiles * fMaxChildren - SkMin32(remainder,
                        (fMaxChildren - fMinChildren) * numTiles);
                if (end > branches->count()) {
                    end = branches->count();
                }

                // Now we sort horizontal strips of rectangles by their x coords
                SkTQSort(branches->begin() + begin, branches->begin() + end - 1, RectLessX());
            }

            for (int j = 0; j < numTiles && currentBranch < branches->count(); ++j) {
                int incrementBy = fMaxChildren;
                if (remainder != 0) {
                    // if need be, omit some nodes to make up for remainder
                    if (remainder <= fMaxChildren - fMinChildren) {
                        incrementBy -= remainder;
                        remainder = 0;
                    } else {
                        incrementBy = fMinChildren;
                        remainder -= fMaxChildren - fMinChildren;
                    }
                }
                Node* n = allocateNode(level);
                n->fNumChildren = 1;
                *n->child(0) = (*branches)[currentBranch];
                Branch b;
                b.fBounds = (*branches)[currentBranch].fBounds;
                b.fChild.subtree = n;
                ++currentBranch;
                for (int k = 1; k < incrementBy && currentBranch < branches->count(); ++k) {
                    b.fBounds.join((*branches)[currentBranch].fBounds);
                    *n->child(k) = (*branches)[currentBranch];
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
}

void SkRTree::validate() const {
#ifdef SK_DEBUG
    if (this->isEmpty()) {
        return;
    }
    SkASSERT(fCount == this->validateSubtree(fRoot.fChild.subtree, fRoot.fBounds, true));
#endif
}

int SkRTree::validateSubtree(Node* root, SkIRect bounds, bool isRoot) const {
    // make sure the pointer is pointing to a valid place
    SkASSERT(fNodes.contains(static_cast<void*>(root)));

    if (isRoot) {
        // If the root of this subtree is the overall root, we have looser standards:
        if (root->isLeaf()) {
            SkASSERT(root->fNumChildren >= 1 && root->fNumChildren <= fMaxChildren);
        } else {
            SkASSERT(root->fNumChildren >= 2 && root->fNumChildren <= fMaxChildren);
        }
    } else {
        SkASSERT(root->fNumChildren >= fMinChildren && root->fNumChildren <= fMaxChildren);
    }

    for (int i = 0; i < root->fNumChildren; ++i) {
        SkASSERT(bounds.contains(root->child(i)->fBounds));
    }

    if (root->isLeaf()) {
        SkASSERT(0 == root->fLevel);
        return root->fNumChildren;
    } else {
        int childCount = 0;
        for (int i = 0; i < root->fNumChildren; ++i) {
            SkASSERT(root->child(i)->fChild.subtree->fLevel == root->fLevel - 1);
            childCount += this->validateSubtree(root->child(i)->fChild.subtree,
                                                root->child(i)->fBounds);
        }
        return childCount;
    }
}

void SkRTree::rewindInserts() {
    SkASSERT(this->isEmpty()); // Currently only supports deferred inserts
    while (!fDeferredInserts.isEmpty() &&
           fClient->shouldRewind(fDeferredInserts.top().fChild.data)) {
        fDeferredInserts.pop();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint32_t get_area(const SkIRect& rect) {
    return rect.width() * rect.height();
}

static inline uint32_t get_overlap(const SkIRect& rect1, const SkIRect& rect2) {
    // I suspect there's a more efficient way of computing this...
    return SkMax32(0, SkMin32(rect1.fRight, rect2.fRight) - SkMax32(rect1.fLeft, rect2.fLeft)) *
           SkMax32(0, SkMin32(rect1.fBottom, rect2.fBottom) - SkMax32(rect1.fTop, rect2.fTop));
}

// Get the margin (aka perimeter)
static inline uint32_t get_margin(const SkIRect& rect) {
    return 2 * (rect.width() + rect.height());
}

static inline uint32_t get_area_increase(const SkIRect& rect1, SkIRect rect2) {
    join_no_empty_check(rect1, &rect2);
    return get_area(rect2) - get_area(rect1);
}

// Expand 'out' to include 'joinWith'
static inline void join_no_empty_check(const SkIRect& joinWith, SkIRect* out) {
    // since we check for empty bounds on insert, we know we'll never have empty rects
    // and we can save the empty check that SkIRect::join requires
    if (joinWith.fLeft < out->fLeft) { out->fLeft = joinWith.fLeft; }
    if (joinWith.fTop < out->fTop) { out->fTop = joinWith.fTop; }
    if (joinWith.fRight > out->fRight) { out->fRight = joinWith.fRight; }
    if (joinWith.fBottom > out->fBottom) { out->fBottom = joinWith.fBottom; }
}
