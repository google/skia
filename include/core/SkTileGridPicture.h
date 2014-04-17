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

class SkTileGridFactory : public SkBBHFactory {
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

    SkTileGridFactory(const TileGridInfo& info) : fInfo(info) { }

    virtual SkBBoxHierarchy* operator()(int width, int height) const SK_OVERRIDE;

private:
    TileGridInfo fInfo;

    typedef SkBBHFactory INHERITED;
};

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

/**
 * Subclass of SkPicture that creates an SkTileGrid. The tile grid has lower recording
 * and playback costs then rTree, but is less effective at eliminating extraneous
 * primitives for arbitrary query rectangles. It is most effective for
 * tiled playback when the tile structure is known at record time.
 */
class SK_API SkTileGridPicture : public SkPicture {
public:
    typedef SkTileGridFactory::TileGridInfo TileGridInfo;

    /**
     * Constructor
     * @param width recording canvas width in device pixels
     * @param height recording canvas height in device pixels
     * @param info description of the tiling layout
     */
    SkTileGridPicture(int width, int height, const SkTileGridFactory::TileGridInfo& info);

    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE;

private:
    int fXTileCount, fYTileCount;
    SkTileGridFactory::TileGridInfo fInfo;

    typedef SkPicture INHERITED;
};

class SkTileGridPictureFactory : public SkPictureFactory {
public:
    SkTileGridPictureFactory(const SkTileGridFactory::TileGridInfo& info) : fInfo(info) { }

    virtual SkPicture* create(int width, int height) SK_OVERRIDE {
        return SkNEW_ARGS(SkTileGridPicture, (width, height, fInfo));
    }

protected:
    SkTileGridFactory::TileGridInfo fInfo;

private:
    typedef SkPictureFactory INHERITED;
};
#endif

#endif
