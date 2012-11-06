
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGrid_DEFINED
#define SkTileGrid_DEFINED

#include "SkBBoxHierarchy.h"

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
    SkTileGrid(int tileWidth, int tileHeight, int xTileCount, int yTileCount);

    virtual ~SkTileGrid();

    /**
     * Insert a data pointer and corresponding bounding box
     * @param data The data pointer, may be NULL
     * @param bounds The bounding box, should not be empty
     * @param defer Ignored, TileArray does not defer insertions
     */
    virtual void insert(void* data, const SkIRect& bounds, bool) SK_OVERRIDE;

    virtual void flushDeferredInserts() SK_OVERRIDE {};

    /**
     * Populate 'results' with data pointers corresponding to bounding boxes that intersect 'query'
     * The query argument is expected to be an exact match to a tile of the grid
     */
    virtual void search(const SkIRect& query, SkTDArray<void*>* results) SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;

    /**
     * Gets the number of insertions
     */
    virtual int getCount() const SK_OVERRIDE;

private:
    SkTDArray<void *>& tile(int x, int y);

    int fTileWidth, fTileHeight, fXTileCount, fYTileCount, fTileCount;
    SkTDArray<void *> *fTileData;
    int fInsertionCount;

    typedef SkBBoxHierarchy INHERITED;
};

#endif

