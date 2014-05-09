/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkQuadTree.h"
#include "SkTSort.h"
#include <stdio.h>

static const int kSplitThreshold = 8;

enum {
    kTopLeft,
    kTopRight,
    kBottomLeft,
    kBottomRight,
};
enum {
    kTopLeft_Bit = 1 << kTopLeft,
    kTopRight_Bit = 1 << kTopRight,
    kBottomLeft_Bit = 1 << kBottomLeft,
    kBottomRight_Bit = 1 << kBottomRight,
};
enum {
    kMaskLeft = kTopLeft_Bit | kBottomLeft_Bit,
    kMaskRight = kTopRight_Bit | kBottomRight_Bit,
    kMaskTop = kTopLeft_Bit | kTopRight_Bit,
    kMaskBottom = kBottomLeft_Bit | kBottomRight_Bit,
};

static U8CPU child_intersect(const SkIRect& query, const SkIPoint& split) {
    // fast quadrant test
    U8CPU intersect = 0xf;
    if (query.fRight <  split.fX) {
        intersect &= ~kMaskRight;
    } else if(query.fLeft >= split.fX) {
        intersect &= ~kMaskLeft;
    }
    if (query.fBottom < split.fY) {
        intersect &= ~kMaskBottom;
    } else if(query.fTop >= split.fY) {
        intersect &= ~kMaskTop;
    }
    return intersect;
}

SkQuadTree::SkQuadTree(const SkIRect& bounds) : fRoot(NULL) {
    SkASSERT((bounds.width() * bounds.height()) > 0);
    fRootBounds = bounds;
}

SkQuadTree::~SkQuadTree() {
}

void SkQuadTree::insert(Node* node, Entry* entry) {
    // does it belong in a child?
    if (NULL != node->fChildren[0]) {
        switch(child_intersect(entry->fBounds, node->fSplitPoint)) {
            case kTopLeft_Bit:
                this->insert(node->fChildren[kTopLeft], entry);
                return;
            case kTopRight_Bit:
                this->insert(node->fChildren[kTopRight], entry);
                return;
            case kBottomLeft_Bit:
                this->insert(node->fChildren[kBottomLeft], entry);
                return;
            case kBottomRight_Bit:
                this->insert(node->fChildren[kBottomRight], entry);
                return;
            default:
                node->fEntries.push(entry);
                return;
        }
    }
    // No children yet, add to this node
    node->fEntries.push(entry);
    // should I split?
    if (node->fEntries.getCount() > kSplitThreshold) {
        this->split(node);
    }
}

void SkQuadTree::split(Node* node) {
    // Build all the children
    node->fSplitPoint = SkIPoint::Make(node->fBounds.centerX(),
                                       node->fBounds.centerY());
    for(int index=0; index<kChildCount; ++index) {
        node->fChildren[index] = fNodePool.acquire();
    }
    node->fChildren[0]->fBounds = SkIRect::MakeLTRB(
        node->fBounds.fLeft,    node->fBounds.fTop,
        node->fSplitPoint.fX,   node->fSplitPoint.fY);
    node->fChildren[1]->fBounds = SkIRect::MakeLTRB(
        node->fSplitPoint.fX,   node->fBounds.fTop,
        node->fBounds.fRight,   node->fSplitPoint.fY);
    node->fChildren[2]->fBounds = SkIRect::MakeLTRB(
        node->fBounds.fLeft,    node->fSplitPoint.fY,
        node->fSplitPoint.fX,   node->fBounds.fBottom);
    node->fChildren[3]->fBounds = SkIRect::MakeLTRB(
        node->fSplitPoint.fX,   node->fSplitPoint.fY,
        node->fBounds.fRight,   node->fBounds.fBottom);
    // reinsert all the entries of this node to allow child trickle
    SkTInternalSList<Entry> entries;
    entries.pushAll(&node->fEntries);
    while(!entries.isEmpty()) {
        this->insert(node, entries.pop());
    }
}

void SkQuadTree::search(Node* node, const SkIRect& query,
                        SkTDArray<void*>* results) const {
    for (Entry* entry = node->fEntries.head(); NULL != entry;
        entry = entry->getSListNext()) {
        if (SkIRect::IntersectsNoEmptyCheck(entry->fBounds, query)) {
            results->push(entry->fData);
        }
    }
    if (NULL == node->fChildren[0]) {
        return;
    }
    U8CPU intersect = child_intersect(query, node->fSplitPoint);
    for(int index=0; index<kChildCount; ++index) {
        if (intersect & (1 << index)) {
            this->search(node->fChildren[index], query, results);
        }
    }
}

void SkQuadTree::clear(Node* node) {
    // first clear the entries of this node
    fEntryPool.releaseAll(&node->fEntries);
    // recurse into and clear all child nodes
    for(int index=0; index<kChildCount; ++index) {
        Node* child = node->fChildren[index];
        node->fChildren[index] = NULL;
        if (NULL != child) {
            this->clear(child);
            fNodePool.release(child);
        }
    }
}

int SkQuadTree::getDepth(Node* node) const {
    int maxDepth = 0;
    if (NULL != node) {
        for(int index=0; index<kChildCount; ++index) {
            maxDepth = SkMax32(maxDepth, getDepth(node->fChildren[index]));
        }
    }
    return maxDepth + 1;
}

void SkQuadTree::insert(void* data, const SkIRect& bounds, bool) {
    if (bounds.isEmpty()) {
        SkASSERT(false);
        return;
    }
    Entry* entry = fEntryPool.acquire();
    entry->fData = data;
    entry->fBounds = bounds;
    if (NULL == fRoot) {
        fDeferred.push(entry);
    } else {
        this->insert(fRoot, entry);
    }
}

void SkQuadTree::search(const SkIRect& query, SkTDArray<void*>* results) {
    SkASSERT(NULL != fRoot);
    SkASSERT(NULL != results);
    if (SkIRect::Intersects(fRootBounds, query)) {
        this->search(fRoot, query, results);
    }
}

void SkQuadTree::clear() {
    this->flushDeferredInserts();
    if (NULL != fRoot) {
        this->clear(fRoot);
        fNodePool.release(fRoot);
        fRoot = NULL;
    }
    SkASSERT(fEntryPool.allocated() == fEntryPool.available());
    SkASSERT(fNodePool.allocated() == fNodePool.available());
}

int SkQuadTree::getDepth() const {
    return this->getDepth(fRoot);
}

void SkQuadTree::rewindInserts() {
    SkASSERT(fClient);
     // Currently only supports deferred inserts
    SkASSERT(NULL == fRoot);
    SkTInternalSList<Entry> entries;
    entries.pushAll(&fDeferred);
    while(!entries.isEmpty()) {
        Entry* entry = entries.pop();
        if (fClient->shouldRewind(entry->fData)) {
            entry->fData = NULL;
            fEntryPool.release(entry);
        } else {
            fDeferred.push(entry);
        }
    }
}

void SkQuadTree::flushDeferredInserts() {
    if (NULL == fRoot) {
        fRoot = fNodePool.acquire();
        fRoot->fBounds = fRootBounds;
    }
    while(!fDeferred.isEmpty()) {
        this->insert(fRoot, fDeferred.pop());
    }
}
