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
#include "SkTInternalSList.h"
#include "SkTObjectPool.h"

/**
 * A QuadTree implementation. In short, it is a tree containing a hierarchy of bounding rectangles
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
     * Quad tree constructor.
     * @param bounds The bounding box for the root of the quad tree.
     *               giving the quad tree bounds that fall outside the root
     *               bounds may result in pathological but correct behavior.
     */
    SkQuadTree(const SkIRect& bounds);

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
    virtual void flushDeferredInserts() SK_OVERRIDE;

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
    virtual int getCount() const SK_OVERRIDE {
        return fEntryPool.allocated() - fEntryPool.available();
    }

    virtual void rewindInserts() SK_OVERRIDE;

private:
    struct Entry {
        Entry() : fData(NULL) {}
        SkIRect fBounds;
        void* fData;
        SK_DECLARE_INTERNAL_SLIST_INTERFACE(Entry);
    };

    static const int kChildCount = 4;

    struct Node {
        Node() {
            for (int index=0; index<kChildCount; ++index) {
                fChildren[index] = NULL;
            }
        }
        SkTInternalSList<Entry> fEntries;
        SkIRect fBounds;
        SkIPoint fSplitPoint; // Only valid if the node has children.
        Node* fChildren[kChildCount];
        SK_DECLARE_INTERNAL_SLIST_ADAPTER(Node, fChildren[0]);
    };

    SkTObjectPool<Entry> fEntryPool;
    SkTObjectPool<Node> fNodePool;
    Node* fRoot;
    SkIRect fRootBounds;
    SkTInternalSList<Entry> fDeferred;

    void insert(Node* node, Entry* entry);
    void split(Node* node);
    void search(Node* node, const SkIRect& query, SkTDArray<void*>* results) const;
    void clear(Node* node);
    int getDepth(Node* node) const;

    typedef SkBBoxHierarchy INHERITED;
};

#endif
