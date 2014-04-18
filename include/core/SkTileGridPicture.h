/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileGridPicture_DEFINED
#define SkTileGridPicture_DEFINED

#ifdef SK_SUPPORT_LEGACY_PICTURE_HEADERS
#include "SkBBHFactory.h"
#endif

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

#include "SkPicture.h"
#include "SkPoint.h"
#include "SkSize.h"

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
