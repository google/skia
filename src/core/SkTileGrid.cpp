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
    , fInvWidth( SkScalarInvert(info.fTileInterval.width()))
    , fInvHeight(SkScalarInvert(info.fTileInterval.height()))
    , fMarginWidth (info.fMargin.fWidth +1)  // Margin is offset by 1 as a provision for AA and
    , fMarginHeight(info.fMargin.fHeight+1)  // to cancel the outset applied by getClipDeviceBounds.
    , fOffset(SkPoint::Make(info.fOffset.fX, info.fOffset.fY))
    , fGridBounds(SkRect::MakeWH(xTiles * info.fTileInterval.width(),
                                 yTiles * info.fTileInterval.height()))
    , fTiles(SkNEW_ARRAY(SkTDArray<unsigned>, xTiles * yTiles)) {}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTiles);
}

void SkTileGrid::reserve(int opCount) {
    if (fXTiles * fYTiles == 0) {
        return;  // A tileless tile grid is nonsensical, but happens in at least cc_unittests.
    }

    // If we assume every op we're about to try to insert() falls within our grid bounds,
    // then every op has to hit at least one tile.  In fact, a quick scan over our small
    // SKP set shows that in the average SKP, each op hits two 256x256 tiles.

    // If we take those observations and further assume the ops are distributed evenly
    // across the picture, we get this guess for number of ops per tile:
    const int opsPerTileGuess = (2 * opCount) / (fXTiles * fYTiles);

    for (SkTDArray<unsigned>* tile = fTiles; tile != fTiles + (fXTiles * fYTiles); tile++) {
        tile->setReserve(opsPerTileGuess);
    }

    // In practice, this heuristic means we'll temporarily allocate about 30% more bytes
    // than if we made no setReserve() calls, but time spent in insert() drops by about 50%.
}

void SkTileGrid::shrinkToFit() {
    for (SkTDArray<unsigned>* tile = fTiles; tile != fTiles + (fXTiles * fYTiles); tile++) {
        tile->shrinkToFit();
    }
}

// Adjustments to user-provided bounds common to both insert() and search().
// Call this after making insert- or search- specific adjustments.
void SkTileGrid::commonAdjust(SkRect* rect) const {
    // Apply our offset.
    rect->offset(fOffset);

    // Scrunch the bounds in just a little to make the right and bottom edges
    // exclusive.  We want bounds of exactly one tile to hit exactly one tile.
    rect->fRight  -= SK_ScalarNearlyZero;
    rect->fBottom -= SK_ScalarNearlyZero;
}

// Convert user-space bounds to grid tiles they cover (LT and RB both inclusive).
void SkTileGrid::userToGrid(const SkRect& user, SkIRect* grid) const {
    grid->fLeft   = SkPin32(user.left()   * fInvWidth , 0, fXTiles - 1);
    grid->fTop    = SkPin32(user.top()    * fInvHeight, 0, fYTiles - 1);
    grid->fRight  = SkPin32(user.right()  * fInvWidth , 0, fXTiles - 1);
    grid->fBottom = SkPin32(user.bottom() * fInvHeight, 0, fYTiles - 1);
}

void SkTileGrid::insert(SkAutoTMalloc<SkRect>* boundsArray, int N) {
    this->reserve(N);

    for (int i = 0; i < N; i++) {
        SkRect bounds = (*boundsArray)[i];
        bounds.outset(fMarginWidth, fMarginHeight);
        this->commonAdjust(&bounds);

        // TODO(mtklein): can we assert this instead to save an intersection in Release mode,
        // or just allow out-of-bound insertions to insert anyway (clamped to nearest tile)?
        if (!SkRect::Intersects(bounds, fGridBounds)) {
            continue;
        }

        SkIRect grid;
        this->userToGrid(bounds, &grid);

        // This is just a loop over y then x.  This compiles to a slightly faster and
        // more compact loop than if we just did fTiles[y * fXTiles + x].push(i).
        SkTDArray<unsigned>* row = &fTiles[grid.fTop * fXTiles + grid.fLeft];
        for (int y = 0; y <= grid.fBottom - grid.fTop; y++) {
            SkTDArray<unsigned>* tile = row;
            for (int x = 0; x <= grid.fRight - grid.fLeft; x++) {
                (tile++)->push(i);
            }
            row += fXTiles;
        }
    }
    this->shrinkToFit();
}

// Number of tiles for which data is allocated on the stack in
// SkTileGrid::search. If malloc becomes a bottleneck, we may consider
// increasing this number. Typical large web page, say 2k x 16k, would
// require 512 tiles of size 256 x 256 pixels.
static const int kStackAllocationTileCount = 1024;

void SkTileGrid::search(const SkRect& originalQuery, SkTDArray<unsigned>* results) const {
    // The inset counteracts the outset that applied in 'insert', which optimizes
    // for lookups of size 'tileInterval + 2 * margin' (aligned with the tile grid).
    SkRect query = originalQuery;
    query.inset(fMarginWidth, fMarginHeight);
    this->commonAdjust(&query);

    // The inset may have inverted the rectangle, so sort().
    // TODO(mtklein): It looks like we only end up with inverted bounds in unit tests
    // that make explicitly inverted queries, not from insetting.  If we can drop support for
    // unsorted bounds (i.e. we don't see them outside unit tests), I think we can drop this.
    query.sort();

    // No intersection check.  We optimize for queries that are in bounds.
    // We're safe anyway: userToGrid() will clamp out-of-bounds queries to nearest tile.
    SkIRect grid;
    this->userToGrid(query, &grid);

    const int tilesHit = (grid.fRight - grid.fLeft + 1) * (grid.fBottom - grid.fTop + 1);
    SkASSERT(tilesHit > 0);

    if (tilesHit == 1) {
        // A performance shortcut.  The merging code below would work fine here too.
        *results = fTiles[grid.fTop * fXTiles + grid.fLeft];
        return;
    }

    // We've got to merge the data in many tiles into a single sorted and deduplicated stream.
    // We do a simple k-way merge based on the value of opIndex.

    // Gather pointers to the starts and ends of the tiles to merge.
    SkAutoSTArray<kStackAllocationTileCount, const unsigned*> starts(tilesHit), ends(tilesHit);
    int i = 0;
    for (int y = grid.fTop; y <= grid.fBottom; y++) {
        for (int x = grid.fLeft; x <= grid.fRight; x++) {
            starts[i] = fTiles[y * fXTiles + x].begin();
            ends[i]   = fTiles[y * fXTiles + x].end();
            i++;
        }
    }

    // Merge tiles into results until they're fully consumed.
    results->reset();
    while (true) {
        // The tiles themselves are already ordered, so the earliest op is at the front of some
        // tile. It may be at the front of several, even all, tiles.
        unsigned earliest = SK_MaxU32;
        for (int i = 0; i < starts.count(); i++) {
            if (starts[i] < ends[i]) {
                earliest = SkTMin(earliest, *starts[i]);
            }
        }

        // If we didn't find an earliest op, there isn't anything left to merge.
        if (SK_MaxU32 == earliest) {
            return;
        }

        // We did find an earliest op. Output it, and step forward every tile that contains it.
        results->push(earliest);
        for (int i = 0; i < starts.count(); i++) {
            if (starts[i] < ends[i] && *starts[i] == earliest) {
                starts[i]++;
            }
        }
    }
}

size_t SkTileGrid::bytesUsed() const {
    size_t byteCount = sizeof(SkTileGrid);

    size_t opCount = 0;
    for (int i = 0; i < fXTiles * fYTiles; i++) {
        opCount += fTiles[i].reserved();
    }
    byteCount += opCount * sizeof(unsigned);

    return byteCount;
}
