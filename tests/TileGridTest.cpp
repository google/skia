
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkTileGrid.h"

enum Tile {
    kTopLeft_Tile = 0x1,
    kTopRight_Tile = 0x2,
    kBottomLeft_Tile = 0x4,
    kBottomRight_Tile = 0x8,

    kAll_Tile = kTopLeft_Tile | kTopRight_Tile | kBottomLeft_Tile | kBottomRight_Tile,
};

class TileGridTest {
public:
    static void verifyTileHits(skiatest::Reporter* reporter, SkIRect rect, uint32_t tileMask) {
        SkTileGrid grid(10, 10, 2, 2);
        grid.insert(NULL, rect, false);
        REPORTER_ASSERT(reporter, grid.tile(0,0).count() ==
            ((tileMask & kTopLeft_Tile)? 1 : 0));
        REPORTER_ASSERT(reporter, grid.tile(1,0).count() ==
            ((tileMask & kTopRight_Tile)? 1 : 0));
        REPORTER_ASSERT(reporter, grid.tile(0,1).count() ==
            ((tileMask & kBottomLeft_Tile)? 1 : 0));
        REPORTER_ASSERT(reporter, grid.tile(1,1).count() ==
            ((tileMask & kBottomRight_Tile)? 1 : 0));
    }

    static void Test(skiatest::Reporter* reporter) {
        // Out of bounds
        verifyTileHits(reporter, SkIRect::MakeXYWH(30, 0, 1, 1),  0);
        verifyTileHits(reporter, SkIRect::MakeXYWH(0, 30, 1, 1),  0);
        verifyTileHits(reporter, SkIRect::MakeXYWH(-10, 0, 1, 1),  0);
        verifyTileHits(reporter, SkIRect::MakeXYWH(0, -10, 1, 1),  0);

        // Dilation for AA consideration
        verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 8, 8),  kTopLeft_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(0, 0, 9, 9),  kAll_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(10, 10, 1, 1),  kAll_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(11, 11, 1, 1),  kBottomRight_Tile);

        // BBoxes that overlap tiles
        verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 10, 1),  kTopLeft_Tile | kTopRight_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 1, 10),  kTopLeft_Tile |
                       kBottomLeft_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(5, 5, 10, 10),  kAll_Tile);
        verifyTileHits(reporter, SkIRect::MakeXYWH(-10, -10, 40, 40),  kAll_Tile);
    }
};


#include "TestClassDef.h"
DEFINE_TESTCLASS("TileGrid", TileGridTestClass, TileGridTest::Test)
