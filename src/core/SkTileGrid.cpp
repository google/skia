
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGrid.h"
#include "SkPictureStateTree.h"

SkTileGrid::SkTileGrid(int xTileCount, int yTileCount, const SkTileGridFactory::TileGridInfo& info) {
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
    fTileData = SkNEW_ARRAY(SkTDArray<void *>, fTileCount);
}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTileData);
}

int SkTileGrid::tileCount(int x, int y) {
    return this->tile(x, y).count();
}

const SkTDArray<void *>& SkTileGrid::tile(int x, int y) const {
    return fTileData[y * fXTileCount + x];
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

static int divide_ceil(int x, int y) {
    return (x + y - 1) / y;
}

// Number of tiles for which data is allocated on the stack in
// SkTileGrid::search. If malloc becomes a bottleneck, we may consider
// increasing this number. Typical large web page, say 2k x 16k, would
// require 512 tiles of size 256 x 256 pixels.
static const int kStackAllocationTileCount = 1024;

void SkTileGrid::search(const SkIRect& query, SkTDArray<void*>* results) const {
    SkIRect adjusted = query;

    // The inset is to counteract the outset that was applied in 'insert'
    // The outset/inset is to optimize for lookups of size
    // 'tileInterval + 2 * margin' that are aligned with the tile grid.
    adjusted.inset(fInfo.fMargin.width(), fInfo.fMargin.height());
    adjusted.offset(fInfo.fOffset);
    adjusted.sort();  // in case the inset inverted the rectangle

    // Convert the query rectangle from device coordinates to tile coordinates
    // by rounding outwards to the nearest tile boundary so that the resulting tile
    // region includes the query rectangle.
    int startX = adjusted.left() / fInfo.fTileInterval.width(),
        startY = adjusted.top()  / fInfo.fTileInterval.height();
    int endX = divide_ceil(adjusted.right(),  fInfo.fTileInterval.width()),
        endY = divide_ceil(adjusted.bottom(), fInfo.fTileInterval.height());

    // Logically, we could pin endX to [startX, fXTileCount], but we force it
    // up to (startX, fXTileCount] to make sure we hit at least one tile.
    // This snaps just-out-of-bounds queries to the neighboring border tile.
    // I don't know if this is an important feature outside of unit tests.
    startX = SkPin32(startX, 0, fXTileCount - 1);
    startY = SkPin32(startY, 0, fYTileCount - 1);
    endX   = SkPin32(endX, startX + 1, fXTileCount);
    endY   = SkPin32(endY, startY + 1, fYTileCount);

    const int tilesHit = (endX - startX) * (endY - startY);
    SkASSERT(tilesHit > 0);

    if (tilesHit == 1) {
        // A performance shortcut.  The merging code below would work fine here too.
        *results = this->tile(startX, startY);
        return;
    }

    // We've got to merge the data in many tiles into a single sorted and deduplicated stream.
    // Each tile itself is already sorted (TODO: assert this while building) so we just need to do
    // a simple k-way merge.

    // Gather pointers to the starts and ends of the tiles to merge.
    SkAutoSTArray<kStackAllocationTileCount, void**> tiles(tilesHit), ends(tilesHit);
    int i = 0;
    for (int x = startX; x < endX; x++) {
        for (int y = startY; y < endY; y++) {
            tiles[i] = fTileData[y * fXTileCount + x].begin();
            ends[i]  = fTileData[y * fXTileCount + x].end();
            i++;
        }
    }

    // Merge tiles into results until they're fully consumed.
    results->reset();
    while (true) {
        // The tiles themselves are already sorted, so the smallest datum is the front of some tile.
        // It may be at the front of several, even all, tiles.
        SkPictureStateTree::Draw* smallest = NULL;
        for (int i = 0; i < tiles.count(); i++) {
            if (tiles[i] < ends[i]) {
                SkPictureStateTree::Draw* candidate =
                    static_cast<SkPictureStateTree::Draw*>(*tiles[i]);
                if (NULL == smallest || (*candidate) < (*smallest)) {
                    smallest = candidate;
                }
            }
        }

        // If we didn't find a smallest datum, there's nothing left to merge.
        if (NULL == smallest) {
            return;
        }

        // We did find a smallest datum. Output it, and step forward in every tile that contains it.
        results->push(smallest);
        for (int i = 0; i < tiles.count(); i++) {
            if (tiles[i] < ends[i] && *tiles[i] == smallest) {
                tiles[i]++;
            }
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
