
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
    enum {
        // Number of tiles for which data is allocated on the stack in
        // SkTileGrid::search. If malloc becomes a bottleneck, we may consider
        // increasing this number. Typical large web page, say 2k x 16k, would
        // require 512 tiles of size 256 x 256 pixels.
        kStackAllocationTileCount = 1024
    };

    SkTileGrid(int xTileCount, int yTileCount, const SkTileGridFactory::TileGridInfo& info);

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
    virtual void search(const SkIRect& query, SkTDArray<void*>* results) const SK_OVERRIDE;

    virtual void clear() SK_OVERRIDE;

    /**
     * Gets the number of insertions
     */
    virtual int getCount() const SK_OVERRIDE;

    virtual int getDepth() const SK_OVERRIDE { return -1; }

    virtual void rewindInserts() SK_OVERRIDE;

    // Used by search() and in SkTileGridHelper implementations
    enum {
        kTileFinished = -1,
    };

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
