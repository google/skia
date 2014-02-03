/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkQuadTree_DEFINED
#define SkQuadTree_DEFINED

#include "SkRect.h"
#include "SkTDArray.h"
#include "SkBBoxHierarchy.h"

/**
 * An QuadTree implementation. In short, it is a tree containing a hierarchy of bounding rectangles
 * in which each internal node has exactly four children.
 *
 * For more details see:
 *
 * http://en.wikipedia.org/wiki/Quadtree
 */
class SkQuadTree : public SkBBoxHierarchy {
public:
    SK_DECLARE_INST_COUNT(SkQuadTree)

    /**
     * Create a new QuadTree
     */
    static SkQuadTree* Create(const SkIRect& bounds);
    virtual ~SkQuadTree();

    /**
     * Insert a node, consisting of bounds and a data value into the tree, if we don't immediately
     * need to use the tree; we may allow the insert to be deferred (this can allow us to bulk-load
     * a large batch of nodes at once, which tends to be faster and produce a better tree).
     *  @param data The data value
     *  @param bounds The corresponding bounding box
     *  @param defer Can this insert be deferred? (this may be ignored)
     */
    virtual void insert(void* data, const SkIRect& bounds, bool defer = false) SK_OVERRIDE;

    /**
     * If any inserts have been deferred, this will add them into the tree
     */
    virtual void flushDeferredInserts() SK_OVERRIDE {}

    /**
     * Given a query rectangle, populates the passed-in array with the elements it intersects
     */
    virtual void search(const SkIRect& query, SkTDArray<void*>* results) SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;

    /**
     * Gets the depth of the tree structure
     */
    virtual int getDepth() const SK_OVERRIDE;

    /**
     * This gets the insertion count (rather than the node count)
     */
    virtual int getCount() const SK_OVERRIDE { return fCount; }

    virtual void rewindInserts() SK_OVERRIDE;

private:
    class QuadTreeNode;

    SkQuadTree(const SkIRect& bounds);

    // This is the count of data elements (rather than total nodes in the tree)
    int fCount;

    QuadTreeNode* fRoot;

    typedef SkBBoxHierarchy INHERITED;
};

#endif
