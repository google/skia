/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileGrid.h"
#include "Sk4x.h"

SkTileGrid::SkTileGrid(int xTiles, int yTiles, const SkTileGridFactory::TileGridInfo& info)
    : fXTiles(xTiles)
    , fNumTiles(xTiles * yTiles)
    , fGridBounds(SkRect::MakeWH(xTiles * info.fTileInterval.width(),
                                 yTiles * info.fTileInterval.height()))
    , fMargin(-info.fMargin.fWidth  - 1,  // Outset margin by 1 as a provision for AA and to
              -info.fMargin.fHeight - 1,  // cancel the outset applied by getClipDeviceBounds().
              +info.fMargin.fWidth  + 1,
              +info.fMargin.fHeight + 1)
    , fOffset(info.fOffset.fX,
              info.fOffset.fY,
              info.fOffset.fX - SK_ScalarNearlyZero,  // We scrunch user-provided bounds in a little
              info.fOffset.fY - SK_ScalarNearlyZero)  // to make right and bottom edges exclusive.
    , fUserToGrid(SkScalarInvert(info.fTileInterval.width()),
                  SkScalarInvert(info.fTileInterval.height()),
                  SkScalarInvert(info.fTileInterval.width()),
                  SkScalarInvert(info.fTileInterval.height()))
    , fGridHigh(fXTiles - 1, yTiles - 1, fXTiles - 1, yTiles - 1)
    , fTiles(SkNEW_ARRAY(SkTDArray<unsigned>, fNumTiles)) {}

SkTileGrid::~SkTileGrid() {
    SkDELETE_ARRAY(fTiles);
}

void SkTileGrid::reserve(unsigned opCount) {
    if (fNumTiles == 0) {
        return;  // A tileless tile grid is nonsensical, but happens in at least cc_unittests.
    }

    // If we assume every op we're about to try to insert() falls within our grid bounds,
    // then every op has to hit at least one tile.  In fact, a quick scan over our small
    // SKP set shows that in the average SKP, each op hits two 256x256 tiles.

    // If we take those observations and further assume the ops are distributed evenly
    // across the picture, we get this guess for number of ops per tile:
    const int opsPerTileGuess = (2 * opCount) / fNumTiles;

    for (SkTDArray<unsigned>* tile = fTiles; tile != fTiles + fNumTiles; tile++) {
        tile->setReserve(opsPerTileGuess);
    }

    // In practice, this heuristic means we'll temporarily allocate about 30% more bytes
    // than if we made no setReserve() calls, but time spent in insert() drops by about 50%.
}

void SkTileGrid::flushDeferredInserts() {
    for (SkTDArray<unsigned>* tile = fTiles; tile != fTiles + fNumTiles; tile++) {
        tile->shrinkToFit();
    }
}

// Convert user-space bounds to grid tiles they cover (LT+RB both inclusive).
// Out of bounds queries are clamped to the single nearest tile.
void SkTileGrid::userToGrid(const Sk4f& user, SkIRect* out) const {
    // Map from user coordinates to grid tile coordinates.
    Sk4f grid = user.multiply(fUserToGrid);

    // Now that we're in grid coordinates, clamp to the grid bounds.
    grid = Sk4f::Max(grid, Sk4f(0,0,0,0));
    grid = Sk4f::Min(grid, fGridHigh);

    // Truncate to integers.
    grid.cast<Sk4i>().store(&out->fLeft);
}

// If the rect is inverted, sort it.
static Sk4f sorted(const Sk4f& ltrb) {
    // To sort:
    //   left, right = minmax(left, right)
    //   top, bottom = minmax(top, bottom)
    Sk4f rblt = ltrb.zwxy(),
         ltlt = Sk4f::Min(ltrb, rblt),  // Holds (2 copies of) new left and top.
         rbrb = Sk4f::Max(ltrb, rblt),  // Holds (2 copies of) new right and bottom.
         sort = Sk4f::XYAB(ltlt, rbrb);
    return sort;
}

// Does this rect intersect the grid?
bool SkTileGrid::intersectsGrid(const Sk4f& ltrb) const {
    SkRect bounds;
    ltrb.store(&bounds.fLeft);
    return SkRect::Intersects(bounds, fGridBounds);
    // TODO: If we can get it fast enough, write intersect using Sk4f.
}

void SkTileGrid::insert(unsigned opIndex, const SkRect& originalBounds, bool) {
    Sk4f bounds = Sk4f(&originalBounds.fLeft).add(fMargin).add(fOffset);
    SkASSERT(sorted(bounds).equal(bounds).allTrue());

    // TODO(mtklein): skip this check and just let out-of-bounds rects insert into nearest tile?
    if (!this->intersectsGrid(bounds)) {
        return;
    }

    SkIRect grid;
    this->userToGrid(bounds, &grid);

    // This is just a loop over y then x.  This compiles to a slightly faster and
    // more compact loop than if we just did fTiles[y * fXTiles + x].push(opIndex).
    SkTDArray<unsigned>* row = &fTiles[grid.fTop * fXTiles + grid.fLeft];
    for (int y = 0; y <= grid.fBottom - grid.fTop; y++) {
        SkTDArray<unsigned>* tile = row;
        for (int x = 0; x <= grid.fRight - grid.fLeft; x++) {
            (tile++)->push(opIndex);
        }
        row += fXTiles;
    }
}

// Number of tiles for which data is allocated on the stack in
// SkTileGrid::search. If malloc becomes a bottleneck, we may consider
// increasing this number. Typical large web page, say 2k x 16k, would
// require 512 tiles of size 256 x 256 pixels.
static const int kStackAllocationTileCount = 1024;

void SkTileGrid::search(const SkRect& originalQuery, SkTDArray<unsigned>* results) const {
    // The .subtract(fMargin) counteracts the .add(fMargin) applied in insert(),
    // which optimizes for lookups of size tileInterval + 2 * margin (aligned with the tile grid).
    // That .subtract(fMargin) may have inverted the rect, so we sort it.
    Sk4f query = sorted(Sk4f(&originalQuery.fLeft).subtract(fMargin).add(fOffset));

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

