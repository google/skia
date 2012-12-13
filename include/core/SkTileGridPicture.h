/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGridPicture_DEFINED
#define SkTileGridPicture_DEFINED

#include "SkPicture.h"

/**
 * Subclass of SkPicture that override the behavior of the
 * kOptimizeForClippedPlayback_RecordingFlag by creating an SkTileGrid
 * structure rather than an R-Tree. The tile grid has lower recording
 * and playback costs, but is less effective at eliminating extraneous
 * primitives for arbitrary query rectangles. It is most effective for
 * tiled playback when the tile structure is known at record time.
 */
class SkTileGridPicture : public SkPicture {
public:
    SkTileGridPicture(int tileWidth, int tileHeight, int width, int height);
    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE;
private:
    int fTileWidth, fTileHeight, fXTileCount, fYTileCount;
};

#endif