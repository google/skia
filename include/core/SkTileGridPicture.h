/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGridPicture_DEFINED
#define SkTileGridPicture_DEFINED

#include "SkPicture.h"
#include "SkPoint.h"
#include "SkSize.h"

/**
 * Subclass of SkPicture that override the behavior of the
 * kOptimizeForClippedPlayback_RecordingFlag by creating an SkTileGrid
 * structure rather than an R-Tree. The tile grid has lower recording
 * and playback costs, but is less effective at eliminating extraneous
 * primitives for arbitrary query rectangles. It is most effective for
 * tiled playback when the tile structure is known at record time.
 */
class SK_API SkTileGridPicture : public SkPicture {
public:
    struct TileGridInfo {
        /** Tile placement interval */
        SkISize  fTileInterval;

        /** Pixel coverage overlap between adjacent tiles */
        SkISize  fMargin;

        /** Offset added to device-space bounding box positions to convert
          * them to tile-grid space. This can be used to adjust the "phase"
          * of the tile grid to match probable query rectangles that will be
          * used to search into the tile grid. As long as the offset is smaller
          * or equal to the margin, there is no need to extend the domain of
          * the tile grid to prevent data loss.
          */
        SkIPoint fOffset;
    };
    /**
     * Constructor
     * @param width recording canvas width in device pixels
     * @param height recording canvas height in device pixels
     * @param info description of the tiling layout
     */
    SkTileGridPicture(int width, int height, const TileGridInfo& info);

    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE;

private:
    int fXTileCount, fYTileCount;
    TileGridInfo fInfo;
};

#endif
