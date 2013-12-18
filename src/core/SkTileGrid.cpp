
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGrid.h"

SkTileGrid::SkTileGrid(int xTileCount, int yTileCount, const SkTileGridPicture::TileGridInfo& info,
    SkTileGridNextDatumFunctionPtr nextDatumFunction)
{
    fXTileCount = xTileCount;
    fYTileCount = yTileCount;
    fInfo = info;
    // Margin is offset by 1 as a provision for AA and
    // to cancel-out the outset applied by getClipDeviceBounds.
    fInfo.fMargin.fHeight++;
    fInfo.fMargin.fWidth++;
    fTileCount = fXTileCount * fYTileCount;
    fInsertionCount = 0;
    fGridBounds = SkIRect::MakeXYWH(0, 0, fInfo.fTileInterval.width() * fXTileCount,
        fInfo.fTileInterval.height() * fYTileCount);
    fNextDatumFunction = nextDatumFunction;
    fTileData = SkNEW_ARRAY(SkTDArray<void *>, fTileCount);
}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTileData);
}

int SkTileGrid::tileCount(int x, int y) {
    return this->tile(x, y).count();
}

SkTDArray<void *>& SkTileGrid::tile(int x, int y) {
    return fTileData[y * fXTileCount + x];
}

void SkTileGrid::insert(void* data, const SkIRect& bounds, bool) {
    SkASSERT(!bounds.isEmpty());
    SkIRect dilatedBounds = bounds;
    dilatedBounds.outset(fInfo.fMargin.width(), fInfo.fMargin.height());
    dilatedBounds.offset(fInfo.fOffset);
    if (!SkIRect::Intersects(dilatedBounds, fGridBounds)) {
        return;
    }

    // Note: SkIRects are non-inclusive of the right() column and bottom() row,
    // hence the "-1"s in the computations of maxTileX and maxTileY.
    int minTileX = SkMax32(SkMin32(dilatedBounds.left() / fInfo.fTileInterval.width(),
        fXTileCount - 1), 0);
    int maxTileX = SkMax32(SkMin32((dilatedBounds.right() - 1) / fInfo.fTileInterval.width(),
        fXTileCount - 1), 0);
    int minTileY = SkMax32(SkMin32(dilatedBounds.top() / fInfo.fTileInterval.height(),
        fYTileCount -1), 0);
    int maxTileY = SkMax32(SkMin32((dilatedBounds.bottom() -1) / fInfo.fTileInterval.height(),
        fYTileCount -1), 0);

    for (int x = minTileX; x <= maxTileX; x++) {
        for (int y = minTileY; y <= maxTileY; y++) {
            this->tile(x, y).push(data);
        }
    }
    fInsertionCount++;
}

void SkTileGrid::search(const SkIRect& query, SkTDArray<void*>* results) {
    SkIRect adjustedQuery = query;
    // The inset is to counteract the outset that was applied in 'insert'
    // The outset/inset is to optimize for lookups of size
    // 'tileInterval + 2 * margin' that are aligned with the tile grid.
    adjustedQuery.inset(fInfo.fMargin.width(), fInfo.fMargin.height());
    adjustedQuery.offset(fInfo.fOffset);
    adjustedQuery.sort();  // in case the inset inverted the rectangle
    // Convert the query rectangle from device coordinates to tile coordinates
    // by rounding outwards to the nearest tile boundary so that the resulting tile
    // region includes the query rectangle. (using truncating division to "floor")
    int tileStartX = adjustedQuery.left() / fInfo.fTileInterval.width();
    int tileEndX = (adjustedQuery.right() + fInfo.fTileInterval.width() - 1) /
        fInfo.fTileInterval.width();
    int tileStartY = adjustedQuery.top() / fInfo.fTileInterval.height();
    int tileEndY = (adjustedQuery.bottom() + fInfo.fTileInterval.height() - 1) /
        fInfo.fTileInterval.height();

    tileStartX = SkPin32(tileStartX, 0, fXTileCount - 1);
    tileEndX = SkPin32(tileEndX, tileStartX+1, fXTileCount);
    tileStartY = SkPin32(tileStartY, 0, fYTileCount - 1);
    tileEndY = SkPin32(tileEndY, tileStartY+1, fYTileCount);

    int queryTileCount = (tileEndX - tileStartX) * (tileEndY - tileStartY);
    SkASSERT(queryTileCount);
    if (queryTileCount == 1) {
        *results = this->tile(tileStartX, tileStartY);
    } else {
        results->reset();
        SkAutoSTArray<kStackAllocationTileCount, int> curPositions(queryTileCount);
        SkAutoSTArray<kStackAllocationTileCount, SkTDArray<void *>*> storage(queryTileCount);
        SkTDArray<void *>** tileRange = storage.get();
        int tile = 0;
        for (int x = tileStartX; x < tileEndX; ++x) {
            for (int y = tileStartY; y < tileEndY; ++y) {
                tileRange[tile] = &this->tile(x, y);
                curPositions[tile] = tileRange[tile]->count() ? 0 : kTileFinished;
                ++tile;
            }
        }
        void *nextElement;
        while(NULL != (nextElement = fNextDatumFunction(tileRange, curPositions))) {
            results->push(nextElement);
        }
    }
}

void SkTileGrid::clear() {
    for (int i = 0; i < fTileCount; i++) {
        fTileData[i].reset();
    }
}

int SkTileGrid::getCount() const {
    return fInsertionCount;
}

void SkTileGrid::rewindInserts() {
    SkASSERT(fClient);
    for (int i = 0; i < fTileCount; ++i) {
        while (!fTileData[i].isEmpty() && fClient->shouldRewind(fTileData[i].top())) {
            fTileData[i].pop();
        }
    }
}
