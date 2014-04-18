/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkPictureRecorder.h"
#include "SkTileGrid.h"
#include "Test.h"

enum Tile {
    kTopLeft_Tile = 0x1,
    kTopRight_Tile = 0x2,
    kBottomLeft_Tile = 0x4,
    kBottomRight_Tile = 0x8,

    kAll_Tile = kTopLeft_Tile | kTopRight_Tile | kBottomLeft_Tile | kBottomRight_Tile,
};

class MockCanvas : public SkCanvas {
public:
    MockCanvas(const SkBitmap& bm) : SkCanvas(bm) {}

    virtual void drawRect(const SkRect& rect, const SkPaint&) {
        // This capture occurs before quick reject.
        fRects.push(rect);
    }

    SkTDArray<SkRect> fRects;
};

static void verifyTileHits(skiatest::Reporter* reporter, SkIRect rect,
                           uint32_t tileMask, int borderPixels = 0) {
    SkTileGridFactory::TileGridInfo info;
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

DEF_TEST(TileGrid_UnalignedQuery, reporter) {
    // Use SkTileGridPicture to generate a SkTileGrid with a helper
    SkTileGridFactory::TileGridInfo info;
    info.fMargin.setEmpty();
    info.fOffset.setZero();
    info.fTileInterval.set(10, 10);
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                    SkIntToScalar(8), SkIntToScalar(8));
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(11), SkIntToScalar(11),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkTileGridFactory factory(info);
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(20, 20, &factory, 0);
    SkPaint paint;
    canvas->drawRect(rect1, paint);
    canvas->drawRect(rect2, paint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkBitmap store;
    store.allocN32Pixels(1, 1);

    // Test parts of top-left tile
    {
        MockCanvas mockCanvas(store);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
    }
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(-7.99f, -7.99f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
    }
    // Corner overlap
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(-9.5f, -9.5f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 2 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[1]);
    }
    // Intersect bottom right tile, but does not overlap rect 2
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(-16.0f, -16.0f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
    }
    // Out of bounds queries, snap to border tiles
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(2.0f, 0.0f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
    }
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(0.0f, 2.0f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
    }
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(-22.0f, -16.0f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
    }
    {
        MockCanvas mockCanvas(store);
        mockCanvas.translate(-16.0f, -22.0f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
    }
}

DEF_TEST(TileGrid_OverlapOffsetQueryAlignment, reporter) {
    // Use SkTileGridPicture to generate a SkTileGrid with a helper
    SkTileGridFactory::TileGridInfo info;
    info.fMargin.set(1, 1);
    info.fOffset.set(-1, -1);
    info.fTileInterval.set(8, 8);

    // rect landing entirely in top left tile
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                    SkIntToScalar(1), SkIntToScalar(1));
    // rect landing entirely in center tile
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(12), SkIntToScalar(12),
                                    SkIntToScalar(1), SkIntToScalar(1));
    // rect landing entirely in bottomright tile
    SkRect rect3 = SkRect::MakeXYWH(SkIntToScalar(19), SkIntToScalar(19),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkTileGridFactory factory(info);
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(20, 20, &factory, 0);
    SkPaint paint;
    canvas->drawRect(rect1, paint);
    canvas->drawRect(rect2, paint);
    canvas->drawRect(rect3, paint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkBitmap tileBitmap;
    tileBitmap.allocN32Pixels(10, 10);
    SkBitmap moreThanATileBitmap;
    moreThanATileBitmap.allocN32Pixels(11, 11);
    SkBitmap tinyBitmap;
    tinyBitmap.allocN32Pixels(2, 2);
    // Test parts of top-left tile
    {
        // The offset should cancel the top and left borders of the top left tile
        // So a look-up at interval 0-10 should be grid aligned,
        MockCanvas mockCanvas(tileBitmap);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
    }
    {
        // Encroaching border by one pixel
        MockCanvas mockCanvas(moreThanATileBitmap);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 2 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[1]);
    }
    {
        // Tile stride is 8 (tileWidth - 2 * border pixels
        // so translating by 8, should make query grid-aligned
        // with middle tile.
        MockCanvas mockCanvas(tileBitmap);
        mockCanvas.translate(SkIntToScalar(-8), SkIntToScalar(-8));
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 1 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
    }
    {
        MockCanvas mockCanvas(tileBitmap);
        mockCanvas.translate(-7.9f, -7.9f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 2 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect1 == mockCanvas.fRects[0]);
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[1]);
    }
    {
        MockCanvas mockCanvas(tileBitmap);
        mockCanvas.translate(-8.1f, -8.1f);
        picture->draw(&mockCanvas);
        REPORTER_ASSERT(reporter, 2 == mockCanvas.fRects.count());
        REPORTER_ASSERT(reporter, rect2 == mockCanvas.fRects[0]);
        REPORTER_ASSERT(reporter, rect3 == mockCanvas.fRects[1]);
    }
    {
        // Regression test for crbug.com/234688
        // Once the 2x2 device region is inset by margin, it yields an empty
        // adjusted region, sitting right on top of the tile boundary.
        MockCanvas mockCanvas(tinyBitmap);
        mockCanvas.translate(-8.0f, -8.0f);
        picture->draw(&mockCanvas);
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
