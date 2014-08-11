
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGrid_DEFINED
#define SkTileGrid_DEFINED

#include "SkBBHFactory.h"
#include "SkBBoxHierarchy.h"
#include "SkPictureStateTree.h"

/**
 * Subclass of SkBBoxHierarchy that stores elements in buckets that correspond
 * to tile regions, disposed in a regular grid.  This is useful when the tile
 * structure that will be use in search() calls is known prior to insertion.
 * Calls to search will return in constant time.
 *
 * Note: Current implementation of search() only supports looking-up regions
 * that are an exact match to a single tile.  Implementation could be augmented
 * to support arbitrary rectangles, but performance would be sub-optimal.
 */
class SkTileGrid : public SkBBoxHierarchy {
public:
    SkTileGrid(int xTileCount, int yTileCount, const SkTileGridFactory::TileGridInfo& info);

    virtual ~SkTileGrid();

    /**
     * Insert a data pointer and corresponding bounding box
     * @param data   The data pointer, may _NOT_ be NULL.
     * @param bounds The bounding box, should not be empty.
     * @param defer  Ignored; SkTileGrid does not defer insertions.
     */
    virtual void insert(void* data, const SkIRect& bounds, bool) SK_OVERRIDE;

    virtual void flushDeferredInserts() SK_OVERRIDE {};

    /**
     * Populate 'results' with data pointers corresponding to bounding boxes that intersect 'query'.
     * This will be fastest if the query is an exact match to a single grid tile.
     */
    virtual void search(const SkIRect& query, SkTDArray<void*>* results) const SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;

    /**
     * Gets the number of insertions
     */
    virtual int getCount() const SK_OVERRIDE;

    virtual int getDepth() const SK_OVERRIDE { return -1; }

    virtual void rewindInserts() SK_OVERRIDE;

    int tileCount(int x, int y);  // For testing only.

private:
    const SkTDArray<void*>& tile(int x, int y) const;
    SkTDArray<void*>& tile(int x, int y);

    int fXTileCount, fYTileCount, fTileCount;
    SkTileGridFactory::TileGridInfo fInfo;
    SkTDArray<void*>* fTileData;
    int fInsertionCount;
    SkIRect fGridBounds;

    typedef SkBBoxHierarchy INHERITED;
};

#endif
