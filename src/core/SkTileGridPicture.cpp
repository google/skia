/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGridPicture.h"

#include "SkPictureStateTree.h"
#include "SkTileGrid.h"


SkTileGridPicture::SkTileGridPicture(int tileWidth, int tileHeight, int width, int height) {
    fTileWidth = tileWidth;
    fTileHeight = tileHeight;
    fXTileCount = (width + tileWidth - 1) / tileWidth;
    fYTileCount = (height + tileHeight - 1) / tileHeight;
}

SkBBoxHierarchy* SkTileGridPicture::createBBoxHierarchy() const {
    return SkNEW_ARGS(SkTileGrid, (fTileWidth, fTileHeight, fXTileCount, fYTileCount,
        SkTileGridNextDatum<SkPictureStateTree::Draw>));
}
