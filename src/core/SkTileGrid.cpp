/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGrid.h"

SkTileGrid::SkTileGrid(int xTiles, int yTiles, const SkTileGridFactory::TileGridInfo& info)
    : fXTiles(xTiles)
    , fYTiles(yTiles)
    , fInfo(info)
    , fCount(0)
    , fTiles(SkNEW_ARRAY(SkTDArray<Entry>, xTiles * yTiles)) {
    // Margin is offset by 1 as a provision for AA and
    // to cancel-out the outset applied by getClipDeviceBounds.
    fInfo.fMargin.fHeight++;
    fInfo.fMargin.fWidth++;
}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTiles);
}

void SkTileGrid::insert(void* data, const SkRect& fbounds, bool) {
    SkASSERT(!fbounds.isEmpty());
    SkIRect dilatedBounds;
    if (fbounds.isLargest()) {
        // Dilating the largest SkIRect will overflow.  Other nearly-largest rects may overflow too,
        // but we don't make active use of them like we do the largest.
        dilatedBounds.setLargest();
    } else {
        fbounds.roundOut(&dilatedBounds);
        dilatedBounds.outset(fInfo.fMargin.width(), fInfo.fMargin.height());
        dilatedBounds.offset(fInfo.fOffset);
    }

    const SkIRect gridBounds =
        { 0, 0, fInfo.fTileInterval.width() * fXTiles, fInfo.fTileInterval.height() * fYTiles };
    if (!SkIRect::Intersects(dilatedBounds, gridBounds)) {
        return;
    }

    // Note: SkIRects are non-inclusive of the right() column and bottom() row,
    // hence the "-1"s in the computations of maxX and maxY.
    int minX = SkMax32(0, SkMin32(dilatedBounds.left() / fInfo.fTileInterval.width(), fXTiles - 1));
    int minY = SkMax32(0, SkMin32(dilatedBounds.top() / fInfo.fTileInterval.height(), fYTiles - 1));
    int maxX = SkMax32(0, SkMin32((dilatedBounds.right()  - 1) / fInfo.fTileInterval.width(),
                                  fXTiles - 1));
    int maxY = SkMax32(0, SkMin32((dilatedBounds.bottom() - 1) / fInfo.fTileInterval.height(),
                                  fYTiles - 1));

    Entry entry = { fCount++, data };
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            fTiles[y * fXTiles + x].push(entry);
        }
    }
}

static int divide_ceil(int x, int y) {
    return (x + y - 1) / y;
}

// Number of tiles for which data is allocated on the stack in
// SkTileGrid::search. If malloc becomes a bottleneck, we may consider
// increasing this number. Typical large web page, say 2k x 16k, would
// require 512 tiles of size 256 x 256 pixels.
static const int kStackAllocationTileCount = 1024;

void SkTileGrid::search(const SkRect& query, SkTDArray<void*>* results) const {
    SkIRect adjusted;
    query.roundOut(&adjusted);

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

    // Logically, we could pin endX to [startX, fXTiles], but we force it
    // up to (startX, fXTiles] to make sure we hit at least one tile.
    // This snaps just-out-of-bounds queries to the neighboring border tile.
    // I don't know if this is an important feature outside of unit tests.
    startX = SkPin32(startX, 0, fXTiles - 1);
    startY = SkPin32(startY, 0, fYTiles - 1);
    endX   = SkPin32(endX, startX + 1, fXTiles);
    endY   = SkPin32(endY, startY + 1, fYTiles);

    const int tilesHit = (endX - startX) * (endY - startY);
    SkASSERT(tilesHit > 0);

    if (tilesHit == 1) {
        // A performance shortcut.  The merging code below would work fine here too.
        const SkTDArray<Entry>& tile = fTiles[startY * fXTiles + startX];
        results->setCount(tile.count());
        for (int i = 0; i < tile.count(); i++) {
            (*results)[i] = tile[i].data;
        }
        return;
    }

    // We've got to merge the data in many tiles into a single sorted and deduplicated stream.
    // We do a simple k-way merge based on the order the data was inserted.

    // Gather pointers to the starts and ends of the tiles to merge.
    SkAutoSTArray<kStackAllocationTileCount, const Entry*> starts(tilesHit), ends(tilesHit);
    int i = 0;
    for (int x = startX; x < endX; x++) {
        for (int y = startY; y < endY; y++) {
            starts[i] = fTiles[y * fXTiles + x].begin();
            ends[i]  = fTiles[y * fXTiles + x].end();
            i++;
        }
    }

    // Merge tiles into results until they're fully consumed.
    results->reset();
    while (true) {
        // The tiles themselves are already ordered, so the earliest is at the front of some tile.
        // It may be at the front of several, even all, tiles.
        const Entry* earliest = NULL;
        for (int i = 0; i < starts.count(); i++) {
            if (starts[i] < ends[i]) {
                if (NULL == earliest || starts[i]->order < earliest->order) {
                    earliest = starts[i];
                }
            }
        }

        // If we didn't find an earliest entry, there isn't anything left to merge.
        if (NULL == earliest) {
            return;
        }

        // We did find an earliest entry. Output it, and step forward every tile that contains it.
        results->push(earliest->data);
        for (int i = 0; i < starts.count(); i++) {
            if (starts[i] < ends[i] && starts[i]->order == earliest->order) {
                starts[i]++;
            }
        }
    }
}

void SkTileGrid::clear() {
    for (int i = 0; i < fXTiles * fYTiles; i++) {
        fTiles[i].reset();
    }
}

void SkTileGrid::rewindInserts() {
    SkASSERT(fClient);
    for (int i = 0; i < fXTiles * fYTiles; i++) {
        while (!fTiles[i].isEmpty() && fClient->shouldRewind(fTiles[i].top().data)) {
            fTiles[i].pop();
        }
    }
}
