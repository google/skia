
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLNameAllocator.h"

/**
 * This is the abstract base class for a nonempty AVL tree that tracks allocated
 * names within the half-open range [fFirst, fEnd). The inner nodes can be
 * sparse (meaning not every name within the range is necessarily allocated),
 * but the bounds are tight, so fFirst *is* guaranteed to be allocated, and so
 * is fEnd - 1.
 */
class GrGLNameAllocator::SparseNameRange : public SkRefCnt {
public:
    virtual ~SparseNameRange() {}

    /**
     * Return the beginning of the range. first() is guaranteed to be allocated.
     *
     * @return The first name in the range.
     */
    GrGLuint first() const { return fFirst; }

    /**
     * Return the end of the range. end() - 1 is guaranteed to be allocated.
     *
     * @return One plus the final name in the range.
     */
    GrGLuint end() const { return fEnd; }

    /**
     * Return the height of the tree. This can only be nonzero at an inner node.
     *
     * @return 0 if the implementation is a leaf node,
     *         The nonzero height of the tree otherwise.
     */
    GrGLuint height() const { return fHeight; }

    /**
     * Allocate a name from strictly inside this range. The call will fail if
     * there is not a free name within.
     *
     * @param outName A pointer that receives the allocated name. outName will
     *                be set to zero if there were no free names within the
     *                range [fFirst, fEnd).
     * @return The resulting SparseNameRange after the allocation. Note that
     *         this call is destructive, so the original SparseNameRange will no
     *         longer be valid afterward. The caller must always update its
     *         pointer with the new SparseNameRange.
     */
    virtual SparseNameRange* SK_WARN_UNUSED_RESULT internalAllocate(GrGLuint* outName) = 0;

    /**
     * Remove the leftmost leaf node from this range (or the entire thing if it
     * *is* a leaf node). This is an internal helper method that is used after
     * an allocation if one contiguous range became adjacent to another. (The
     * range gets removed so the one immediately before can be extended,
     * collapsing the two into one.)
     *
     * @param removedCount A pointer that receives the size of the contiguous
                           range that was removed.
     * @return The resulting SparseNameRange after the removal (or NULL if it
     *         became empty). Note that this call is destructive, so the
     *         original SparseNameRange will no longer be valid afterward. The
     *         caller must always update its pointer with the new
     *         SparseNameRange.
     */
    virtual SparseNameRange* SK_WARN_UNUSED_RESULT removeLeftmostContiguousRange(GrGLuint* removedCount) = 0;

    /**
     * Append adjacent allocated names to the end of this range. This operation
     * does not affect the structure of the tree. The caller is responsible for
     * ensuring the new names won't overlap sibling ranges, if any.
     *
     * @param count The number of adjacent names to append.
     * @return The first name appended.
     */
    virtual GrGLuint appendNames(GrGLuint count) = 0;

    /**
     * Prepend adjacent allocated names behind the beginning of this range. This
     * operation does not affect the structure of the tree. The caller is
     * responsible for ensuring the new names won't overlap sibling ranges, if
     * any.
     *
     * @param count The number of adjacent names to prepend.
     * @return The final name prepended (the one with the lowest value).
     */
    virtual GrGLuint prependNames(GrGLuint count) = 0;

    /**
     * Free a name so it is no longer tracked as allocated. If the name is at
     * the very beginning or very end of the range, the boundaries [fFirst, fEnd)
     * will be tightened.
     *
     * @param name The name to free. Not-allocated names are silently ignored
     *             the same way they are in the OpenGL spec.
     * @return The resulting SparseNameRange after the free (or NULL if it
     *         became empty). Note that this call is destructive, so the
     *         original SparseNameRange will no longer be valid afterward. The
     *         caller must always update its pointer with the new
     *         SparseNameRange.
     */
    virtual SparseNameRange* SK_WARN_UNUSED_RESULT free(GrGLuint name) = 0;

protected:
    SparseNameRange* takeRef() {
      this->ref();
      return this;
    }

    GrGLuint fFirst;
    GrGLuint fEnd;
    GrGLuint fHeight;
};

/**
 * This class is the SparseNameRange implementation for an inner node. It is an
 * AVL tree with non-null, non-adjacent left and right children.
 */
class GrGLNameAllocator::SparseNameTree : public SparseNameRange {
public:
    SparseNameTree(SparseNameRange* left, SparseNameRange* right)
        : fLeft(left),
          fRight(right) {
        SkASSERT(fLeft.get());
        SkASSERT(fRight.get());
        this->updateStats();
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT internalAllocate(GrGLuint* outName) override {
        // Try allocating the range inside fLeft's internal gaps.
        fLeft.reset(fLeft->internalAllocate(outName));
        if (0 != *outName) {
            this->updateStats();
            return this->rebalance();
        }

        if (fLeft->end() + 1 == fRight->first()) {
            // It closed the gap between fLeft and fRight; merge.
            GrGLuint removedCount;
            fRight.reset(fRight->removeLeftmostContiguousRange(&removedCount));
            *outName = fLeft->appendNames(1 + removedCount);
            if (NULL == fRight.get()) {
                return fLeft.detach();
            }
            this->updateStats();
            return this->rebalance();
        }

        // There is guaranteed to be a gap between fLeft and fRight, and the
        // "size 1" case has already been covered.
        SkASSERT(fLeft->end() + 1 < fRight->first());
        *outName = fLeft->appendNames(1);
        return this->takeRef();
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT removeLeftmostContiguousRange(GrGLuint* removedCount) override {
        fLeft.reset(fLeft->removeLeftmostContiguousRange(removedCount));
        if (NULL == fLeft) {
            return fRight.detach();
        }
        this->updateStats();
        return this->rebalance();
    }

    GrGLuint appendNames(GrGLuint count) override {
        SkASSERT(fEnd + count > fEnd); // Check for integer wrap.
        GrGLuint name = fRight->appendNames(count);
        SkASSERT(fRight->end() == fEnd + count);
        this->updateStats();
        return name;
    }

    GrGLuint prependNames(GrGLuint count) override {
        SkASSERT(fFirst > count); // We can't allocate at or below 0.
        GrGLuint name = fLeft->prependNames(count);
        SkASSERT(fLeft->first() == fFirst - count);
        this->updateStats();
        return name;
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT free(GrGLuint name) override {
        if (name < fLeft->end()) {
            fLeft.reset(fLeft->free(name));
            if (NULL == fLeft) {
                // fLeft became empty after the free.
                return fRight.detach();
            }
            this->updateStats();
            return this->rebalance();
        } else {
            fRight.reset(fRight->free(name));
            if (NULL == fRight) {
                // fRight became empty after the free.
                return fLeft.detach();
            }
            this->updateStats();
            return this->rebalance();
        }
    }

private:
    typedef SkAutoTUnref<SparseNameRange> SparseNameTree::* ChildRange;

    SparseNameRange* SK_WARN_UNUSED_RESULT rebalance() {
        if (fLeft->height() > fRight->height() + 1) {
            return this->rebalanceImpl<&SparseNameTree::fLeft, &SparseNameTree::fRight>();
        }
        if (fRight->height() > fLeft->height() + 1) {
            return this->rebalanceImpl<&SparseNameTree::fRight, &SparseNameTree::fLeft>();
        }
        return this->takeRef();
    }

    /**
     * Rebalance the tree using rotations, as described in the AVL algorithm:
     * http://en.wikipedia.org/wiki/AVL_tree#Insertion
     */
    template<ChildRange Tall, ChildRange Short>
    SparseNameRange* SK_WARN_UNUSED_RESULT rebalanceImpl() {
        // We should be calling rebalance() enough that the tree never gets more
        // than one rotation off balance.
        SkASSERT(2 == (this->*Tall)->height() - (this->*Short)->height());

        // Ensure we are in the 'Left Left' or 'Right Right' case:
        // http://en.wikipedia.org/wiki/AVL_tree#Insertion
        SparseNameTree* tallChild = static_cast<SparseNameTree*>((this->*Tall).get());
        if ((tallChild->*Tall)->height() < (tallChild->*Short)->height()) {
            (this->*Tall).reset(tallChild->rotate<Short, Tall>());
        }

        // Perform a rotation to balance the tree.
        return this->rotate<Tall, Short>();
    }

    /**
     * Perform a node rotation, as described in the AVL algorithm:
     * http://en.wikipedia.org/wiki/AVL_tree#Insertion
     */
    template<ChildRange Tall, ChildRange Short>
    SparseNameRange* SK_WARN_UNUSED_RESULT rotate() {
        SparseNameTree* newRoot = static_cast<SparseNameTree*>((this->*Tall).detach());

        (this->*Tall).reset((newRoot->*Short).detach());
        this->updateStats();

        (newRoot->*Short).reset(this->takeRef());
        newRoot->updateStats();

        return newRoot;
    }

    void updateStats() {
        SkASSERT(fLeft->end() < fRight->first()); // There must be a gap between left and right.
        fFirst = fLeft->first();
        fEnd = fRight->end();
        fHeight = 1 + SkMax32(fLeft->height(), fRight->height());
    }

    SkAutoTUnref<SparseNameRange> fLeft;
    SkAutoTUnref<SparseNameRange> fRight;
};

/**
 * This class is the SparseNameRange implementation for a leaf node. It just a
 * contiguous range of allocated names.
 */
class GrGLNameAllocator::ContiguousNameRange : public SparseNameRange {
public:
    ContiguousNameRange(GrGLuint first, GrGLuint end) {
        SkASSERT(first < end);
        fFirst = first;
        fEnd = end;
        fHeight = 0;
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT internalAllocate(GrGLuint* outName) override {
        *outName = 0; // No internal gaps, we are contiguous.
        return this->takeRef();
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT removeLeftmostContiguousRange(GrGLuint* removedCount) override {
        *removedCount = fEnd - fFirst;
        return NULL;
    }

    GrGLuint appendNames(GrGLuint count) override {
        SkASSERT(fEnd + count > fEnd); // Check for integer wrap.
        GrGLuint name = fEnd;
        fEnd += count;
        return name;
    }

    GrGLuint prependNames(GrGLuint count) override {
        SkASSERT(fFirst > count); // We can't allocate at or below 0.
        fFirst -= count;
        return fFirst;
    }

    SparseNameRange* SK_WARN_UNUSED_RESULT free(GrGLuint name) override {
        if (name < fFirst || name >= fEnd) {
          // Not-allocated names are silently ignored.
          return this->takeRef();
        }

        if (fFirst == name) {
            ++fFirst;
            return (fEnd == fFirst) ? NULL : this->takeRef();
        }

        if (fEnd == name + 1) {
            --fEnd;
            return this->takeRef();
        }

        SparseNameRange* left = SkNEW_ARGS(ContiguousNameRange, (fFirst, name));
        SparseNameRange* right = this->takeRef();
        fFirst = name + 1;
        return SkNEW_ARGS(SparseNameTree, (left, right));
    }
};

GrGLNameAllocator::GrGLNameAllocator(GrGLuint firstName, GrGLuint endName)
    : fFirstName(firstName),
      fEndName(endName) {
    SkASSERT(firstName > 0);
    SkASSERT(endName > firstName);
}

GrGLNameAllocator::~GrGLNameAllocator() {
}

GrGLuint GrGLNameAllocator::allocateName() {
    if (NULL == fAllocatedNames.get()) {
        fAllocatedNames.reset(SkNEW_ARGS(ContiguousNameRange, (fFirstName, fFirstName + 1)));
        return fFirstName;
    }

    if (fAllocatedNames->first() > fFirstName) {
        return fAllocatedNames->prependNames(1);
    }

    GrGLuint name;
    fAllocatedNames.reset(fAllocatedNames->internalAllocate(&name));
    if (0 != name) {
        return name;
    }

    if (fAllocatedNames->end() < fEndName) {
        return fAllocatedNames->appendNames(1);
    }

    // Out of names.
    return 0;
}

void GrGLNameAllocator::free(GrGLuint name) {
    if (!fAllocatedNames.get()) {
      // Not-allocated names are silently ignored.
      return;
    }

    fAllocatedNames.reset(fAllocatedNames->free(name));
}
