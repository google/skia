/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGridPicture.h"

#include "SkPictureStateTree.h"
#include "SkTileGrid.h"

SkTileGridPicture::SkTileGridPicture(int width, int height, const TileGridInfo& info) {
    SkASSERT(info.fMargin.width() >= 0);
    SkASSERT(info.fMargin.height() >= 0);
    fInfo = info;
    // Note: SkIRects are non-inclusive of the right() column and bottom() row.
    // For example, an SkIRect at 0,0 with a size of (1,1) will only have
    // content at pixel (0,0) and will report left=0 and right=1, hence the
    // "-1"s below.
    fXTileCount = (width + info.fTileInterval.width() - 1) / info.fTileInterval.width();
    fYTileCount = (height + info.fTileInterval.height() - 1) / info.fTileInterval.height();
}

SkBBoxHierarchy* SkTileGridPicture::createBBoxHierarchy() const {
    return SkNEW_ARGS(SkTileGrid, (fXTileCount, fYTileCount, fInfo,
         SkTileGridNextDatum<SkPictureStateTree::Draw>));
}
