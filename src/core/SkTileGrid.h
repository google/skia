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

    virtual void insert(SkAutoTMalloc<SkRect>* boundsArray, int N) SK_OVERRIDE;
    virtual void search(const SkRect& query, SkTDArray<unsigned>* results) const SK_OVERRIDE;

    // For testing.
    int tileCount(int x, int y) { return fTiles[y * fXTiles + x].count(); }

    virtual size_t bytesUsed() const SK_OVERRIDE;

private:
    void reserve(int);
    void shrinkToFit();

    void commonAdjust(SkRect*) const;
    void userToGrid(const SkRect&, SkIRect* grid) const;

    const int fXTiles, fYTiles;
    const SkScalar fInvWidth, fInvHeight;
    const SkScalar fMarginWidth, fMarginHeight;
    const SkPoint fOffset;
    const SkRect  fGridBounds;

    // (fXTiles * fYTiles) SkTDArrays, each listing ops overlapping that tile in order.
    SkTDArray<unsigned>* fTiles;

    typedef SkBBoxHierarchy INHERITED;
};

#endif
