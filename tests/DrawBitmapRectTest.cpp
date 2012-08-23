
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkShader.h"

#ifdef SK_SCALAR_IS_FLOAT
static void assert_ifDrawnTo(skiatest::Reporter* reporter,
                             const SkBitmap& bm, bool shouldBeDrawn) {
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (shouldBeDrawn) {
                if (0 == *bm.getAddr32(x, y)) {
                    REPORTER_ASSERT(reporter, false);
                    return;
                }
            } else {
                // should not be drawn
                if (*bm.getAddr32(x, y)) {
                    REPORTER_ASSERT(reporter, false);
                    return;
                }
            }
        }
    }
}

static void test_wacky_bitmapshader(skiatest::Reporter* reporter,
                                    int width, int height, bool shouldBeDrawn) {
    SkBitmap dev;
    dev.setConfig(SkBitmap::kARGB_8888_Config, 0x56F, 0x4f6);
    dev.allocPixels();
    dev.eraseColor(0);  // necessary, so we know if we draw to it

    SkMatrix matrix;

    SkCanvas c(dev);
    matrix.setAll(SkFloatToScalar(-119.34097f),
                  SkFloatToScalar(-43.436558f),
                  SkFloatToScalar(93489.945f),
                  SkFloatToScalar(43.436558f),
                  SkFloatToScalar(-119.34097f),
                  SkFloatToScalar(123.98426f),
                  0, 0, SK_Scalar1);
    c.concat(matrix);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bm.allocPixels();
    bm.eraseColor(SK_ColorRED);

    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);
    matrix.setAll(SkFloatToScalar(0.0078740157f),
                  0,
                  SkIntToScalar(249),
                  0,
                  SkFloatToScalar(0.0078740157f),
                  SkIntToScalar(239),
                  0, 0, SK_Scalar1);
    s->setLocalMatrix(matrix);

    SkPaint paint;
    paint.setShader(s)->unref();

    SkRect r = SkRect::MakeXYWH(681, 239, 695, 253);
    c.drawRect(r, paint);

    assert_ifDrawnTo(reporter, dev, shouldBeDrawn);
}
#endif

/*
 *  Original bug was asserting that the matrix-proc had generated a (Y) value
 *  that was out of range. This led (in the release build) to the sampler-proc
 *  reading memory out-of-bounds of the original bitmap.
 *
 *  We were numerically overflowing our 16bit coordinates that we communicate
 *  between these two procs. The fixes was in two parts:
 *
 *  1. Just don't draw bitmaps larger than 64K-1 in width or height, since we
 *     can't represent those coordinates in our transport format (yet).
 *  2. Perform an unsigned shift during the calculation, so we don't get
 *     sign-extension bleed when packing the two values (X,Y) into our 32bit
 *     slot.
 *
 *  This tests exercises the original setup, plus 3 more to ensure that we can,
 *  in fact, handle bitmaps at 64K-1 (assuming we don't exceed the total
 *  memory allocation limit).
 */
static void test_giantrepeat_crbug118018(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    static const struct {
        int fWidth;
        int fHeight;
        bool fExpectedToDraw;
    } gTests[] = {
        { 0x1b294, 0x7f,  false },   // crbug 118018 (width exceeds 64K)
        { 0xFFFF, 0x7f,    true },   // should draw, test max width
        { 0x7f, 0xFFFF,    true },   // should draw, test max height
        { 0xFFFF, 0xFFFF, false },   // allocation fails (too much RAM)
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        test_wacky_bitmapshader(reporter,
                                gTests[i].fWidth, gTests[i].fHeight,
                                gTests[i].fExpectedToDraw);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void test_nan_antihair(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 20, 20);
    bm.allocPixels();

    SkCanvas canvas(bm);

    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(10, SK_ScalarNaN);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    // before our fix to SkScan_Antihair.cpp to check for integral NaN (0x800...)
    // this would trigger an assert/crash.
    //
    // see rev. 3558
    canvas.drawPath(path, paint);
}

static bool check_for_all_zeros(const SkBitmap& bm) {
    SkAutoLockPixels alp(bm);

    size_t count = bm.width() * bm.bytesPerPixel();
    for (int y = 0; y < bm.height(); y++) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(bm.getAddr(0, y));
        for (size_t i = 0; i < count; i++) {
            if (ptr[i]) {
                return false;
            }
        }
    }
    return true;
}

static const int gWidth = 256;
static const int gHeight = 256;

static void create(SkBitmap* bm, SkBitmap::Config config, SkColor color) {
    bm->setConfig(config, gWidth, gHeight);
    bm->allocPixels();
    bm->eraseColor(color);
}

static void TestDrawBitmapRect(skiatest::Reporter* reporter) {
    SkBitmap src, dst;

    create(&src, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    create(&dst, SkBitmap::kARGB_8888_Config, 0);

    SkCanvas canvas(dst);

    SkIRect srcR = { gWidth, 0, gWidth + 16, 16 };
    SkRect  dstR = { 0, 0, SkIntToScalar(16), SkIntToScalar(16) };

    canvas.drawBitmapRect(src, &srcR, dstR, NULL);

    // ensure that we draw nothing if srcR does not intersect the bitmap
    REPORTER_ASSERT(reporter, check_for_all_zeros(dst));

    test_nan_antihair(reporter);
    test_giantrepeat_crbug118018(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawBitmapRect", TestDrawBitmapRectClass, TestDrawBitmapRect)
