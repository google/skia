
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkTileGrid.h"
#include "SkTileGridPicture.h"
#include "SkCanvas.h"
#include "SkDevice.h"

enum Tile {
    kTopLeft_Tile = 0x1,
    kTopRight_Tile = 0x2,
    kBottomLeft_Tile = 0x4,
    kBottomRight_Tile = 0x8,

    kAll_Tile = kTopLeft_Tile | kTopRight_Tile | kBottomLeft_Tile | kBottomRight_Tile,
};

namespace {
class MockCanvas : public SkCanvas {
public:
    MockCanvas(SkDevice* device) : SkCanvas(device)
    {}

    virtual void drawRect(const SkRect& rect, const SkPaint& paint)
    {
        // This capture occurs before quick reject.
        fRects.push(rect);
    }

    SkTDArray<SkRect> fRects;
};
}

class TileGridTest {
public:
    static void verifyTileHits(skiatest::Reporter* reporter, SkIRect rect, uint32_t tileMask) {
        SkTileGrid grid(10, 10, 2, 2, NULL);
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

    static void TestUnalignedQuery(skiatest::Reporter* reporter) {
        // Use SkTileGridPicture to generate a SkTileGrid with a helper
        SkTileGridPicture picture(10, 10, 20, 20);
        SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
            SkIntToScalar(8), SkIntToScalar(8));
        SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(11), SkIntToScalar(11),
            SkIntToScalar(1), SkIntToScalar(1));
        SkCanvas* canvas = picture.beginRecording(20, 20, SkPicture::kOptimizeForClippedPlayback_RecordingFlag);
        SkPaint paint;
        canvas->drawRect(rect1, paint);
        canvas->drawRect(rect2, paint);
        picture.endRecording();

        SkBitmap store;
        store.setConfig(SkBitmap::kARGB_8888_Config, 1, 1);
        store.allocPixels();

        // Test parts of top-left tile
        {
            SkDevice device(store);
            MockCanvas mockCanvas(&device);
            picture.draw(&mockCanvas);
            REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
            REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
        }
        {
            SkDevice device(store);
            MockCanvas mockCanvas(&device);
            mockCanvas.translate(SkFloatToScalar(-7.99f), SkFloatToScalar(-7.99f));
            picture.draw(&mockCanvas);
            REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
            REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
        }
        // Corner overlap
        {
            SkDevice device(store);
            MockCanvas mockCanvas(&device);
            mockCanvas.translate(SkFloatToScalar(-9.5f), SkFloatToScalar(-9.5f));
            picture.draw(&mockCanvas);
            REPORTER_ASSERT(reporter, 2 == mockCanvas.fRects.count());
            REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
            REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[1]);
        }
        // Intersect bottom right tile, but does not overlap rect 2
        {
            SkDevice device(store);
            MockCanvas mockCanvas(&device);
            mockCanvas.translate(SkFloatToScalar(-16.0f), SkFloatToScalar(-16.0f));
            picture.draw(&mockCanvas);
            REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
            REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
        }
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

        TestUnalignedQuery(reporter);
    }
};


#include "TestClassDef.h"
DEFINE_TESTCLASS("TileGrid", TileGridTestClass, TileGridTest::Test)
