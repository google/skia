/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDiscardableMemoryPool.h"
#include "SkImageGeneratorPriv.h"
#include "SkMatrixUtils.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "Test.h"

// A BitmapFactory that always fails when asked to return pixels.
class FailureImageGenerator : public SkImageGenerator {
public:
    FailureImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(100, 100)) {}
protected:
    // default onGetPixels() returns kUnimplemented, which is what we want.
};

// crbug.com/295895
// Crashing in skia when a pixelref fails in lockPixels
//
static void test_faulty_pixelref(skiatest::Reporter* reporter) {
    // need a cache, but don't expect to use it, so the budget is not critical
    SkAutoTUnref<SkDiscardableMemoryPool> pool(
        SkDiscardableMemoryPool::Create(10 * 1000, NULL));
    SkBitmap bm;
    bool success = SkInstallDiscardablePixelRef(SkNEW(FailureImageGenerator), NULL, &bm, pool);
    REPORTER_ASSERT(reporter, success);
    // now our bitmap has a pixelref, but we know it will fail to lock

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(200, 200));
    SkCanvas* canvas = surface->getCanvas();

    const SkFilterQuality levels[] = {
        kNone_SkFilterQuality,
        kLow_SkFilterQuality,
        kMedium_SkFilterQuality,
        kHigh_SkFilterQuality,
    };

    SkPaint paint;
    canvas->scale(2, 2);    // need a scale, otherwise we may ignore filtering
    for (size_t i = 0; i < SK_ARRAY_COUNT(levels); ++i) {
        paint.setFilterQuality(levels[i]);
        canvas->drawBitmap(bm, 0, 0, &paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void rand_matrix(SkMatrix* mat, SkRandom& rand, unsigned mask) {
    mat->setIdentity();
    if (mask & SkMatrix::kTranslate_Mask) {
        mat->postTranslate(rand.nextSScalar1(), rand.nextSScalar1());
    }
    if (mask & SkMatrix::kScale_Mask) {
        mat->postScale(rand.nextSScalar1(), rand.nextSScalar1());
    }
    if (mask & SkMatrix::kAffine_Mask) {
        mat->postRotate(rand.nextSScalar1() * 360);
    }
    if (mask & SkMatrix::kPerspective_Mask) {
        mat->setPerspX(rand.nextSScalar1());
        mat->setPerspY(rand.nextSScalar1());
    }
}

static void rand_size(SkISize* size, SkRandom& rand) {
    size->set(rand.nextU() & 0xFFFF, rand.nextU() & 0xFFFF);
}

static bool treat_as_sprite(const SkMatrix& mat, const SkISize& size,
                            unsigned bits) {
    return SkTreatAsSprite(mat, size.width(), size.height(), bits);
}

static void test_treatAsSprite(skiatest::Reporter* reporter) {
    const unsigned bilerBits = kSkSubPixelBitsForBilerp;

    SkMatrix mat;
    SkISize  size;
    SkRandom rand;

    // assert: translate-only no-filter can always be treated as sprite
    for (int i = 0; i < 1000; ++i) {
        rand_matrix(&mat, rand, SkMatrix::kTranslate_Mask);
        for (int j = 0; j < 1000; ++j) {
            rand_size(&size, rand);
            REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, 0));
        }
    }

    // assert: rotate/perspect is never treated as sprite
    for (int i = 0; i < 1000; ++i) {
        rand_matrix(&mat, rand, SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask);
        for (int j = 0; j < 1000; ++j) {
            rand_size(&size, rand);
            REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, 0));
            REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, bilerBits));
        }
    }

    size.set(500, 600);

    const SkScalar tooMuchSubpixel = 100.1f;
    mat.setTranslate(tooMuchSubpixel, 0);
    REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, bilerBits));
    mat.setTranslate(0, tooMuchSubpixel);
    REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, bilerBits));

    const SkScalar tinySubPixel = 100.02f;
    mat.setTranslate(tinySubPixel, 0);
    REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, bilerBits));
    mat.setTranslate(0, tinySubPixel);
    REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, bilerBits));

    const SkScalar twoThirds = SK_Scalar1 * 2 / 3;
    const SkScalar bigScale = (size.width() + twoThirds) / size.width();
    mat.setScale(bigScale, bigScale);
    REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, false));
    REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, bilerBits));

    const SkScalar oneThird = SK_Scalar1 / 3;
    const SkScalar smallScale = (size.width() + oneThird) / size.width();
    mat.setScale(smallScale, smallScale);
    REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, false));
    REPORTER_ASSERT(reporter, !treat_as_sprite(mat, size, bilerBits));

    const SkScalar oneFortyth = SK_Scalar1 / 40;
    const SkScalar tinyScale = (size.width() + oneFortyth) / size.width();
    mat.setScale(tinyScale, tinyScale);
    REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, false));
    REPORTER_ASSERT(reporter, treat_as_sprite(mat, size, bilerBits));
}

static void assert_ifDrawnTo(skiatest::Reporter* reporter,
                             const SkBitmap& bm, bool shouldBeDrawn) {
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (shouldBeDrawn) {
                if (SK_ColorTRANSPARENT == *bm.getAddr32(x, y)) {
                    REPORTER_ASSERT(reporter, false);
                    return;
                }
            } else {
                // should not be drawn
                if (SK_ColorTRANSPARENT != *bm.getAddr32(x, y)) {
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
    dev.allocN32Pixels(0x56F, 0x4f6);
    dev.eraseColor(SK_ColorTRANSPARENT);  // necessary, so we know if we draw to it

    SkMatrix matrix;

    SkCanvas c(dev);
    matrix.setAll(-119.34097f,
                  -43.436558f,
                  93489.945f,
                  43.436558f,
                  -119.34097f,
                  123.98426f,
                  0, 0, SK_Scalar1);
    c.concat(matrix);

    SkBitmap bm;
    if (bm.tryAllocN32Pixels(width, height)) {
        // allow this to fail silently, to test the code downstream
    }
    bm.eraseColor(SK_ColorRED);

    matrix.setAll(0.0078740157f,
                  0,
                  SkIntToScalar(249),
                  0,
                  0.0078740157f,
                  SkIntToScalar(239),
                  0, 0, SK_Scalar1);
    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode, &matrix);

    SkPaint paint;
    paint.setShader(s)->unref();

    SkRect r = SkRect::MakeXYWH(681, 239, 695, 253);
    c.drawRect(r, paint);

    assert_ifDrawnTo(reporter, dev, shouldBeDrawn);
}

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
}

///////////////////////////////////////////////////////////////////////////////

static void test_nan_antihair() {
    SkBitmap bm;
    bm.allocN32Pixels(20, 20);

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

static void create(SkBitmap* bm, SkColor color) {
    bm->allocN32Pixels(gWidth, gHeight);
    bm->eraseColor(color);
}

DEF_TEST(DrawBitmapRect, reporter) {
    SkBitmap src, dst;

    create(&src, 0xFFFFFFFF);
    create(&dst, 0);

    SkCanvas canvas(dst);

    SkIRect srcR = { gWidth, 0, gWidth + 16, 16 };
    SkRect  dstR = { 0, 0, SkIntToScalar(16), SkIntToScalar(16) };

    canvas.drawBitmapRect(src, srcR, dstR, NULL);

    // ensure that we draw nothing if srcR does not intersect the bitmap
    REPORTER_ASSERT(reporter, check_for_all_zeros(dst));

    test_nan_antihair();
    test_giantrepeat_crbug118018(reporter);

    test_treatAsSprite(reporter);
    test_faulty_pixelref(reporter);
}
