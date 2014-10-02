
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBoxHierarchy_DEFINED
#define SkBBoxHierarchy_DEFINED

#include "SkRect.h"
#include "SkTDArray.h"
#include "SkRefCnt.h"

/**
 * Interface for a spatial data structure that associates user data with axis-aligned
 * bounding boxes, and allows efficient retrieval of intersections with query rectangles.
 */
class SkBBoxHierarchy : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkBBoxHierarchy)

    SkBBoxHierarchy() {}

    /**
     * Insert opIndex and corresponding bounding box
     * @param opIndex Any value, will be returned in order.
     * @param bounds The bounding box, should not be empty
     * @param defer Whether or not it is acceptable to delay insertion of this element (building up
     *        an entire spatial data structure at once is often faster and produces better
     *        structures than repeated inserts) until flushDeferredInserts is called or the first
     *        search.
     */
    virtual void insert(unsigned opIndex, const SkRect& bounds, bool defer = false) = 0;

    /**
     * If any insertions have been deferred, this forces them to be inserted
     */
    virtual void flushDeferredInserts() = 0;

    /**
     * Populate results with sorted opIndex corresponding to bounding boxes that intersect query.
     */
    virtual void search(const SkRect& query, SkTDArray<unsigned>* results) const = 0;

private:
    typedef SkRefCnt INHERITED;
};

#endif
