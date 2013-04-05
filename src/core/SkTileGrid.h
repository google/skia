
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGrid_DEFINED
#define SkTileGrid_DEFINED

#include "SkBBoxHierarchy.h"
#include "SkPictureStateTree.h"
#include "SkTileGridPicture.h" // for TileGridInfo

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
    typedef void* (*SkTileGridNextDatumFunctionPtr)(SkTDArray<void*>** tileData, SkTDArray<int>& tileIndices);

    SkTileGrid(int xTileCount, int yTileCount, const SkTileGridPicture::TileGridInfo& info,
        SkTileGridNextDatumFunctionPtr nextDatumFunction);

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

    virtual void rewindInserts() SK_OVERRIDE;

    // Used by search() and in SkTileGridHelper implementations
    enum {
        kTileFinished = -1,
    };
private:
    SkTDArray<void*>& tile(int x, int y);

    int fXTileCount, fYTileCount, fTileCount;
    SkTileGridPicture::TileGridInfo fInfo;
    SkTDArray<void*>* fTileData;
    int fInsertionCount;
    SkIRect fGridBounds;
    SkTileGridNextDatumFunctionPtr fNextDatumFunction;

    friend class TileGridTest;
    typedef SkBBoxHierarchy INHERITED;
};

/**
 * Generic implementation for SkTileGridNextDatumFunctionPtr. user code may instantiate
 * this template to get a valid SkTileGridNextDatumFunction implementation
 *
 * Returns the next element of tileData[i][tileIndices[i]] for all i and advances
 * tileIndices[] past them. The order in which data are returned by successive
 * calls to this method must reflect the order in which the were originally
 * recorded into the tile grid.
 *
 * \param tileData array of pointers to arrays of tile data
 * \param tileIndices per-tile data indices, indices are incremented for tiles that contain
 *     the next datum.
 * \tparam T a type to which it is safe to cast a datum and that has an operator <
 *     such that 'a < b' is true if 'a' was inserted into the tile grid before 'b'.
 */
template <typename T>
void* SkTileGridNextDatum(SkTDArray<void*>** tileData, SkTDArray<int>& tileIndices) {
    T* minVal = NULL;
    int tileCount = tileIndices.count();
    int minIndex = tileCount;
    int maxIndex = 0;
    // Find the next Datum; track where it's found so we reduce the size of the second loop.
    for (int tile = 0; tile < tileCount; ++tile) {
        int pos = tileIndices[tile];
        if (pos != SkTileGrid::kTileFinished) {
            T* candidate = (T*)(*tileData[tile])[pos];
            if (NULL == minVal || (*candidate) < (*minVal)) {
                minVal = candidate;
                minIndex = tile;
                maxIndex = tile;
            } else if (!((*minVal) < (*candidate))) {
                // We don't require operator==; if !(candidate<minVal) && !(minVal<candidate),
                // candidate==minVal and we have to add this tile to the range searched.
                maxIndex = tile;
            }
        }
    }
    // Increment indices past the next datum
    if (minVal != NULL) {
        for (int tile = minIndex; tile <= maxIndex; ++tile) {
            int pos = tileIndices[tile];
            if (pos != SkTileGrid::kTileFinished && (*tileData[tile])[pos] == minVal) {
                if (++(tileIndices[tile]) >= tileData[tile]->count()) {
                    tileIndices[tile] = SkTileGrid::kTileFinished;
                }
            }
        }
        return minVal;
    }
    return NULL;
}

#endif
