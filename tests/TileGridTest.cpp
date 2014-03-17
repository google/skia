/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkTileGrid.h"
#include "SkTileGridPicture.h"
#include "Test.h"

enum Tile {
    kTopLeft_Tile = 0x1,
    kTopRight_Tile = 0x2,
    kBottomLeft_Tile = 0x4,
    kBottomRight_Tile = 0x8,

    kAll_Tile = kTopLeft_Tile | kTopRight_Tile | kBottomLeft_Tile | kBottomRight_Tile,
};

static void verifyTileHits(skiatest::Reporter* reporter, SkIRect rect,
                           uint32_t tileMask, int borderPixels = 0) {
    SkTileGridPicture::TileGridInfo info;
    info.fMargin.set(borderPixels, borderPixels);
    info.fOffset.setZero();
    info.fTileInterval.set(10 - 2 * borderPixels, 10 - 2 * borderPixels);
    SkTileGrid grid(2, 2, info, NULL);
    grid.insert(NULL, rect, false);
    REPORTER_ASSERT(reporter, grid.tileCount(0, 0) ==
                    ((tileMask & kTopLeft_Tile)? 1 : 0));
    REPORTER_ASSERT(reporter, grid.tileCount(1, 0) ==
                    ((tileMask & kTopRight_Tile)? 1 : 0));
    REPORTER_ASSERT(reporter, grid.tileCount(0, 1) ==
                    ((tileMask & kBottomLeft_Tile)? 1 : 0));
    REPORTER_ASSERT(reporter, grid.tileCount(1, 1) ==
                    ((tileMask & kBottomRight_Tile)? 1 : 0));
}

static SkIRect query(float x, float y, float w, float h) {
    // inflate for the margin++ in tilegrid
    SkRect bounds = SkRect::MakeXYWH(x, y, w, h);
    SkIRect r;
    bounds.roundOut(&r);
    r.outset(1, 1); // to counteract the inset in SkTileGrid::search
    return r;
}


DEF_TEST(TileGrid_UnalignedQuery, reporter) {
    SkTileGridPicture::TileGridInfo info;
    info.fMargin.setEmpty();
    info.fOffset.setZero();
    info.fTileInterval.set(10, 10);
    SkIRect rect1 = SkIRect::MakeXYWH(0, 0, 8, 8);
    SkIRect rect2 = SkIRect::MakeXYWH(11, 11, 1, 1);
    SkTileGrid grid(2, 2, info, SkTileGridNextDatum<SkPictureStateTree::Draw>);
    grid.insert(&rect1, rect1, true);
    grid.insert(&rect2, rect2, true);

    // Test parts of top-left tile
    {
        SkTDArray<void*> rects;
        grid.search(query(0.0f, 0.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(7.99f, 7.99f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
    }
    // Corner overlap
    {
        SkTDArray<void*> rects;
        grid.search(query(9.5f, 9.5f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 2 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    // Intersect bottom right tile, but does not overlap rect 2
    {
        SkTDArray<void*> rects;
        grid.search(query(16.0f, 16.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    // Out of bounds queries, snap to border tiles
    {
        SkTDArray<void*> rects;
        grid.search(query(-2.0f, 0.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(0.0f, -2.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(22.0f, 16.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(16.0f, 22.0f, 1.0f, 1.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
}

DEF_TEST(TileGrid_OverlapOffsetQueryAlignment, reporter) {
    // Use SkTileGridPicture to generate a SkTileGrid with a helper
    SkTileGridPicture::TileGridInfo info;
    info.fMargin.set(1, 1);
    info.fOffset.set(-1, -1);
    info.fTileInterval.set(8, 8);

    // rect landing entirely in top left tile
    SkIRect rect1 = SkIRect::MakeXYWH(0, 0, 1, 1);
    // rect landing entirely in center tile
    SkIRect rect2 = SkIRect::MakeXYWH(12, 12, 1, 1);
    // rect landing entirely in bottomright tile
    SkIRect rect3 = SkIRect::MakeXYWH(19, 19, 1, 1);
    SkTileGrid grid(3, 3, info, SkTileGridNextDatum<SkPictureStateTree::Draw>);
    grid.insert(&rect1, rect1, true);
    grid.insert(&rect2, rect2, true);
    grid.insert(&rect3, rect3, true);

    // Test parts of top-left tile
    {
        // The offset should cancel the top and left borders of the top left tile
        // So a look-up at interval 0-10 should be grid aligned,
        SkTDArray<void*> rects;
        grid.search(query(0.0f, 0.0f, 10.0f, 10.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
    }
    {
        // Encroaching border by one pixel
        SkTDArray<void*> rects;
        grid.search(query(0.0f, 0.0f, 11.0f, 11.0f), &rects);
        REPORTER_ASSERT(reporter, 2 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    {
        // Tile stride is 8 (tileWidth - 2 * border pixels
        // so translating by 8, should make query grid-aligned
        // with middle tile.
        SkTDArray<void*> rects;
        grid.search(query(8.0f, 8.0f, 10.0f, 10.0f), &rects);
        REPORTER_ASSERT(reporter, 1 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(7.9f, 7.9f, 10.0f, 10.0f), &rects);
        REPORTER_ASSERT(reporter, 2 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect1) >= 0);
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
    }
    {
        SkTDArray<void*> rects;
        grid.search(query(8.1f, 8.1f, 10.0f, 10.0f), &rects);
        REPORTER_ASSERT(reporter, 2 == rects.count());
        REPORTER_ASSERT(reporter, rects.find(&rect2) >= 0);
        REPORTER_ASSERT(reporter, rects.find(&rect3) >= 0);
    }
    {
        // Regression test for crbug.com/234688
        // Once the 2x2 device region is inset by margin, it yields an empty
        // adjusted region, sitting right on top of the tile boundary.
        SkTDArray<void*> rects;
        grid.search(query(8.0f, 8.0f, 2.0f, 2.0f), &rects);
        // This test passes by not asserting. We do not validate the rects recorded
        // because the result is numerically unstable (floating point equality).
        // The content of any one of the four tiles of the tilegrid would be a valid
        // result since any bbox that covers the center point of the canvas will be
        // recorded in all four tiles.
    }
}

DEF_TEST(TileGrid, reporter) {
    // Out of bounds
    verifyTileHits(reporter, SkIRect::MakeXYWH(30, 0, 1, 1),  0);
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, 30, 1, 1),  0);
    verifyTileHits(reporter, SkIRect::MakeXYWH(-10, 0, 1, 1),  0);
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, -10, 1, 1),  0);

    // Dilation for AA consideration
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 9, 9),  kTopLeft_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 10, 10),  kAll_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(9, 9, 1, 1),  kAll_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(10, 10, 1, 1),  kAll_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(11, 11, 1, 1),  kBottomRight_Tile);

    // BorderPixels
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 6, 6),  kTopLeft_Tile, 1);
    verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 7, 7),  kAll_Tile, 1);
    verifyTileHits(reporter, SkIRect::MakeXYWH(9, 9, 1, 1),  kAll_Tile, 1);
    verifyTileHits(reporter, SkIRect::MakeXYWH(10, 10, 1, 1),  kBottomRight_Tile, 1);
    verifyTileHits(reporter, SkIRect::MakeXYWH(17, 17, 1, 1),  kBottomRight_Tile, 1);

    // BBoxes that overlap tiles
    verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 10, 1),  kTopLeft_Tile | kTopRight_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 1, 10),  kTopLeft_Tile |
                   kBottomLeft_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 10, 10),  kAll_Tile);
    verifyTileHits(reporter, SkIRect::MakeXYWH(-10, -10, 40, 40),  kAll_Tile);
}
