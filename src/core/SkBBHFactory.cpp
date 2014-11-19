/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBHFactory.h"
#include "SkRTree.h"
#include "SkTileGrid.h"

SkBBoxHierarchy* SkRTreeFactory::operator()(const SkRect& bounds) const {
    SkScalar aspectRatio = bounds.width() / bounds.height();
    return SkNEW_ARGS(SkRTree, (aspectRatio));
}

SkBBoxHierarchy* SkTileGridFactory::operator()(const SkRect& bounds) const {
    SkASSERT(fInfo.fMargin.width() >= 0);
    SkASSERT(fInfo.fMargin.height() >= 0);

    // We want a conservative answer for the size...
    const SkIRect ibounds = bounds.roundOut();
    const int width = ibounds.width();
    const int height = ibounds.height();

    // Note: SkIRects are non-inclusive of the right() column and bottom() row.
    // For example, an SkIRect at 0,0 with a size of (1,1) will only have
    // content at pixel (0,0) and will report left=0 and right=1, hence the
    // "-1"s below.
    int xTileCount = (width + fInfo.fTileInterval.width() - 1) / fInfo.fTileInterval.width();
    int yTileCount = (height + fInfo.fTileInterval.height() - 1) / fInfo.fTileInterval.height();
    return SkNEW_ARGS(SkTileGrid, (xTileCount, yTileCount, fInfo));
}
