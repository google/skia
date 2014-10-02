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

/**
 * Subclass of SkBBoxHierarchy that stores elements in buckets that correspond
 * to tile regions, disposed in a regular grid.  This is useful when the tile
 * structure that will be use in search() calls is known prior to insertion.
 */
class SkTileGrid : public SkBBoxHierarchy {
public:
    SkTileGrid(int xTiles, int yTiles, const SkTileGridFactory::TileGridInfo& info);

    virtual ~SkTileGrid();

    /**
     * Insert a opIndex value and corresponding bounding box
     * @param opIndex
     * @param bounds The bounding box, should not be empty.
     * @param defer  Ignored; SkTileGrid does not defer insertions.
     */
    virtual void insert(unsigned opIndex, const SkRect& bounds, bool) SK_OVERRIDE;

    virtual void flushDeferredInserts() SK_OVERRIDE {};

    /**
     * Populate 'results' with opIndexes corresponding to bounding boxes that intersect 'query'.
     * This will be fastest if the query is an exact match to a single grid tile.
     */
    virtual void search(const SkRect& query, SkTDArray<unsigned>* results) const SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;

    virtual int getCount() const SK_OVERRIDE { return fCount; }

    virtual int getDepth() const SK_OVERRIDE { return -1; }

    // For testing.
    int tileCount(int x, int y) { return fTiles[y * fXTiles + x].count(); }

private:
    const int fXTiles, fYTiles;
    SkTileGridFactory::TileGridInfo fInfo;
    size_t fCount;

    // (fXTiles * fYTiles) SkTDArrays, each listing ops overlapping that tile in order.
    SkTDArray<unsigned>* fTiles;

    typedef SkBBoxHierarchy INHERITED;
};

#endif
