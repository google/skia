/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkQuadTree.h"
#include "SkTSort.h"
#include <stdio.h>
#include <vector>

static const int kSplitThreshold = 8;
static const int kMinDimensions = 128;

SkQuadTree::SkQuadTree(const SkIRect& bounds)
    : fEntryCount(0)
    , fRoot(NULL) {
    SkASSERT((bounds.width() * bounds.height()) > 0);
    fRoot = fNodePool.acquire();
    fRoot->fBounds = bounds;
}

SkQuadTree::~SkQuadTree() {
}

SkQuadTree::Node* SkQuadTree::pickChild(Node* node,
                                        const SkIRect& bounds) const {
    // is it entirely to the left?
    int index = 0;
    if (bounds.fRight < node->fSplitPoint.fX) {
        // Inside the left side
    } else if(bounds.fLeft >= node->fSplitPoint.fX) {
        // Inside the right side
        index |= 1;
    } else {
        // Not inside any children
        return NULL;
    }
    if (bounds.fBottom < node->fSplitPoint.fY) {
        // Inside the top side
    } else if(bounds.fTop >= node->fSplitPoint.fY) {
        // Inside the bottom side
        index |= 2;
    } else {
        // Not inside any children
        return NULL;
    }
    return node->fChildren[index];
}

void SkQuadTree::insert(Node* node, Entry* entry) {
    // does it belong in a child?
    if (NULL != node->fChildren[0]) {
        Node* child = pickChild(node, entry->fBounds);
        if (NULL != child) {
            insert(child, entry);
        } else {
            node->fEntries.push(entry);
        }
        return;
    }
    // No children yet, add to this node
    node->fEntries.push(entry);
    // should I split?
    if (node->fEntries.getCount() < kSplitThreshold) {
        return;
    }

    if ((node->fBounds.width() < kMinDimensions) ||
        (node->fBounds.height() < kMinDimensions)) {
        return;
    }

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
        insert(node, entries.pop());
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
    // fast quadrant test
    bool left = true;
    bool right = true;
    if (query.fRight < node->fSplitPoint.fX) {
        right = false;
    } else if(query.fLeft >= node->fSplitPoint.fX) {
        left = false;
    }
    bool top = true;
    bool bottom = true;
    if (query.fBottom < node->fSplitPoint.fY) {
        bottom = false;
    } else if(query.fTop >= node->fSplitPoint.fY) {
        top = false;
    }
    // search all the active quadrants
    if (top && left) {
        search(node->fChildren[0], query, results);
    }
    if (top && right) {
        search(node->fChildren[1], query, results);
    }
    if (bottom && left) {
        search(node->fChildren[2], query, results);
    }
    if (bottom && right) {
        search(node->fChildren[3], query, results);
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
            clear(child);
            fNodePool.release(child);
        }
    }
}

int SkQuadTree::getDepth(Node* node) const {
    int maxDepth = 0;
    if (NULL != node->fChildren[0]) {
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
    ++fEntryCount;
    if (fRoot->fEntries.isEmpty() && (NULL == fRoot->fChildren[0])) {
        fDeferred.push(entry);
    } else {
        insert(fRoot, entry);
    }
}

void SkQuadTree::search(const SkIRect& query, SkTDArray<void*>* results) {
    SkASSERT(fDeferred.isEmpty());
    SkASSERT(NULL != results);
    if (SkIRect::Intersects(fRoot->fBounds, query)) {
        search(fRoot, query, results);
    }
}

void SkQuadTree::clear() {
    fEntryCount = 0;
    clear(fRoot);
}

int SkQuadTree::getDepth() const {
    return getDepth(fRoot);
}

void SkQuadTree::rewindInserts() {
    SkASSERT(fClient);
     // Currently only supports deferred inserts
    SkASSERT(fRoot->fEntries.isEmpty() && fRoot->fChildren[0] == NULL);
    SkTInternalSList<Entry> entries;
    entries.pushAll(&fDeferred);
    while(!entries.isEmpty()) {
        Entry* entry = entries.pop();
        if (fClient->shouldRewind(entry->fData)) {
            entry->fData = NULL;
            fEntryPool.release(entry);
            --fEntryCount;
        } else {
            fDeferred.push(entry);
        }
    }
}

void SkQuadTree::flushDeferredInserts() {
    while(!fDeferred.isEmpty()) {
        insert(fRoot, fDeferred.pop());
    }
}
