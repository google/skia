
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRTree_DEFINED
#define SkRTree_DEFINED

#include "SkRect.h"
#include "SkTDArray.h"
#include "SkChunkAlloc.h"
#include "SkBBoxHierarchy.h"

/**
 * An R-Tree implementation. In short, it is a balanced n-ary tree containing a hierarchy of
 * bounding rectangles.
 *
 * Much like a B-Tree it maintains balance by enforcing minimum and maximum child counts, and
 * splitting nodes when they become overfull. Unlike B-trees, however, we're using spatial data; so
 * there isn't a canonical ordering to use when choosing insertion locations and splitting
 * distributions. A variety of heuristics have been proposed for these problems; here, we're using
 * something resembling an R*-tree, which attempts to minimize area and overlap during insertion,
 * and aims to minimize a combination of margin, overlap, and area when splitting.
 *
 * One detail that is thus far unimplemented that may improve tree quality is attempting to remove
 * and reinsert nodes when they become full, instead of immediately splitting (nodes that may have
 * been placed well early on may hurt the tree later when more nodes have been added; removing
 * and reinserting nodes generally helps reduce overlap and make a better tree). Deletion of nodes
 * is also unimplemented.
 *
 * For more details see:
 *
 *  Beckmann, N.; Kriegel, H. P.; Schneider, R.; Seeger, B. (1990). "The R*-tree:
 *      an efficient and robust access method for points and rectangles"
 *
 * It also supports bulk-loading from a batch of bounds and values; if you don't require the tree
 * to be usable in its intermediate states while it is being constructed, this is significantly
 * quicker than individual insertions and produces more consistent trees.
 */
class SkRTree : public SkBBoxHierarchy {
public:
    SK_DECLARE_INST_COUNT(SkRTree)

    /**
     * Create a new R-Tree with specified min/max child counts.
     * The child counts are valid iff:
     * - (max + 1) / 2 >= min (splitting an overfull node must be enough to populate 2 nodes)
     * - min < max
     * - min > 0
     * - max < SK_MaxU16
     * If you have some prior information about the distribution of bounds you're expecting, you
     * can provide an optional aspect ratio parameter. This allows the bulk-load algorithm to create
     * better proportioned tiles of rectangles.
     */
    static SkRTree* Create(int minChildren, int maxChildren, SkScalar aspectRatio = 1,
            bool orderWhenBulkLoading = true);
    virtual ~SkRTree();

    /**
     * Insert a node, consisting of bounds and a data value into the tree, if we don't immediately
     * need to use the tree; we may allow the insert to be deferred (this can allow us to bulk-load
     * a large batch of nodes at once, which tends to be faster and produce a better tree).
     *  @param data The data value
     *  @param bounds The corresponding bounding box
     *  @param defer Can this insert be deferred? (this may be ignored)
     */
    virtual void insert(void* data, const SkRect& bounds, bool defer = false) SK_OVERRIDE;

    /**
     * If any inserts have been deferred, this will add them into the tree
     */
    virtual void flushDeferredInserts() SK_OVERRIDE;

    /**
     * Given a query rectangle, populates the passed-in array with the elements it intersects
     */
    virtual void search(const SkRect& query, SkTDArray<void*>* results) const SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;
    bool isEmpty() const { return 0 == fCount; }

    /**
     * Gets the depth of the tree structure
     */
    virtual int getDepth() const SK_OVERRIDE {
        return this->isEmpty() ? 0 : fRoot.fChild.subtree->fLevel + 1;
    }

    /**
     * This gets the insertion count (rather than the node count)
     */
    virtual int getCount() const SK_OVERRIDE { return fCount; }

    virtual void rewindInserts() SK_OVERRIDE;

private:

    struct Node;

    /**
     * A branch of the tree, this may contain a pointer to another interior node, or a data value
     */
    struct Branch {
        union {
            Node* subtree;
            void* data;
        } fChild;
        SkIRect fBounds;
    };

    /**
     * A node in the tree, has between fMinChildren and fMaxChildren (the root is a special case)
     */
    struct Node {
        uint16_t fNumChildren;
        uint16_t fLevel;
        bool isLeaf() { return 0 == fLevel; }
        // Since we want to be able to pick min/max child counts at runtime, we assume the creator
        // has allocated sufficient space directly after us in memory, and index into that space
        Branch* child(size_t index) {
            return reinterpret_cast<Branch*>(this + 1) + index;
        }
    };

    typedef int32_t SkIRect::*SortSide;

    // Helper for sorting our children arrays by sides of their rects
    struct RectLessThan {
        RectLessThan(SkRTree::SortSide side) : fSide(side) { }
        bool operator()(const SkRTree::Branch lhs, const SkRTree::Branch rhs) const {
            return lhs.fBounds.*fSide < rhs.fBounds.*fSide;
        }
    private:
        const SkRTree::SortSide fSide;
    };

    struct RectLessX {
        bool operator()(const SkRTree::Branch lhs, const SkRTree::Branch rhs) {
            return ((lhs.fBounds.fRight - lhs.fBounds.fLeft) >> 1) <
                   ((rhs.fBounds.fRight - lhs.fBounds.fLeft) >> 1);
        }
    };

    struct RectLessY {
        bool operator()(const SkRTree::Branch lhs, const SkRTree::Branch rhs) {
            return ((lhs.fBounds.fBottom - lhs.fBounds.fTop) >> 1) <
                   ((rhs.fBounds.fBottom - lhs.fBounds.fTop) >> 1);
        }
    };

    SkRTree(int minChildren, int maxChildren, SkScalar aspectRatio, bool orderWhenBulkLoading);

    /**
     * Recursively descend the tree to find an insertion position for 'branch', updates
     * bounding boxes on the way up.
     */
    Branch* insert(Node* root, Branch* branch, uint16_t level = 0);

    int chooseSubtree(Node* root, Branch* branch);
    SkIRect computeBounds(Node* n);
    int distributeChildren(Branch* children);
    void search(Node* root, const SkIRect query, SkTDArray<void*>* results) const;

    /**
     * This performs a bottom-up bulk load using the STR (sort-tile-recursive) algorithm, this
     * seems to generally produce better, more consistent trees at significantly lower cost than
     * repeated insertions.
     *
     * This consumes the input array.
     *
     * TODO: Experiment with other bulk-load algorithms (in particular the Hilbert pack variant,
     * which groups rects by position on the Hilbert curve, is probably worth a look). There also
     * exist top-down bulk load variants (VAMSplit, TopDownGreedy, etc).
     */
    Branch bulkLoad(SkTDArray<Branch>* branches, int level = 0);

    void validate() const;
    int validateSubtree(Node* root, SkIRect bounds, bool isRoot = false) const;

    const int fMinChildren;
    const int fMaxChildren;
    const size_t fNodeSize;

    // This is the count of data elements (rather than total nodes in the tree)
    int fCount;

    Branch fRoot;
    SkChunkAlloc fNodes;
    SkTDArray<Branch> fDeferredInserts;
    SkScalar fAspectRatio;
    bool fSortWhenBulkLoading;

    Node* allocateNode(uint16_t level);

    typedef SkBBoxHierarchy INHERITED;
};

#endif
