/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBHFactory.h"
#include "SkPictureStateTree.h"
#include "SkQuadTree.h"
#include "SkRTree.h"
#include "SkTileGrid.h"


SkBBoxHierarchy* SkQuadTreeFactory::operator()(int width, int height) const {
    return SkNEW_ARGS(SkQuadTree, (SkIRect::MakeWH(width, height)));
}

SkBBoxHierarchy* SkRTreeFactory::operator()(int width, int height) const {
    // These values were empirically determined to produce reasonable
    // performance in most cases.
    static const int kRTreeMinChildren = 6;
    static const int kRTreeMaxChildren = 11;

    SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(width),
                                       SkIntToScalar(height));
    bool sortDraws = false;  // Do not sort draw calls when bulk loading.

    return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                           aspectRatio, sortDraws);
}

SkBBoxHierarchy* SkTileGridFactory::operator()(int width, int height) const {
    SkASSERT(fInfo.fMargin.width() >= 0);
    SkASSERT(fInfo.fMargin.height() >= 0);
    // Note: SkIRects are non-inclusive of the right() column and bottom() row.
    // For example, an SkIRect at 0,0 with a size of (1,1) will only have
    // content at pixel (0,0) and will report left=0 and right=1, hence the
    // "-1"s below.
    int xTileCount = (width + fInfo.fTileInterval.width() - 1) / fInfo.fTileInterval.width();
    int yTileCount = (height + fInfo.fTileInterval.height() - 1) / fInfo.fTileInterval.height();
    return SkNEW_ARGS(SkTileGrid, (xTileCount, yTileCount, fInfo,
                                    SkTileGridNextDatum<SkPictureStateTree::Draw>));
}
