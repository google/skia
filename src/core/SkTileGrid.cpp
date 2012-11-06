
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGrid.h"

SkTileGrid::SkTileGrid(int tileWidth, int tileHeight, int xTileCount, int yTileCount)
{
    fTileWidth = tileWidth;
    fTileHeight = tileHeight;
    fXTileCount = xTileCount;
    fYTileCount = yTileCount;
    fTileCount = fXTileCount * fYTileCount;
    fInsertionCount = 0;

    fTileData = SkNEW_ARRAY(SkTDArray<void *>, fTileCount);
}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTileData);
}

SkTDArray<void *>& SkTileGrid::tile(int x, int y) {
    return fTileData[y * fXTileCount + x];
}

void SkTileGrid::insert(void* data, const SkIRect& bounds, bool) {
    SkASSERT(!bounds.isEmpty());
    SkIRect dilatedBounds = bounds;
    dilatedBounds.outset(1,1); // Consideration for filtering and AA

    int minTileX = SkMax32(SkMin32(dilatedBounds.left() / fTileWidth, fXTileCount - 1), 0);
    int maxTileX = SkMax32(SkMin32(dilatedBounds.right() / fTileWidth, fXTileCount - 1), 0);
    int minTileY = SkMax32(SkMin32(dilatedBounds.top() / fTileHeight, fYTileCount -1), 0);
    int maxTileY = SkMax32(SkMin32(dilatedBounds.bottom() / fTileHeight, fYTileCount -1), 0);

    for (int x = minTileX; x <= maxTileX; x++) {
        for (int y = minTileY; y <= maxTileY; y++) {
            this->tile(x, y).push(data);
        }
    }
    fInsertionCount++;
}

void SkTileGrid::search(const SkIRect& query, SkTDArray<void*>* results) {
    SkASSERT(query.width() == fTileWidth + 2 && query.height() == fTileHeight + 2);
    SkASSERT((query.left() + 1) % fTileWidth == 0);
    SkASSERT((query.top() + 1) % fTileHeight == 0);
    SkASSERT(results);
    // The +1 is to compensate for the outset in applied SkCanvas::getClipBounds
    *results = this->tile((query.left() + 1) / fTileWidth, (query.top() + 1) / fTileHeight);
}

void SkTileGrid::clear() {
    for (int i = 0; i < fTileCount; i++) {
        fTileData[i].reset();
    }
}

int SkTileGrid::getCount() const {
    return fInsertionCount;
}


