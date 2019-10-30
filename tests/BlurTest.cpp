/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMath.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlurDrawLooper.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/private/SkFloatBits.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkBlurPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMathPriv.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/GrContextFactory.h"

#include <math.h>
#include <string.h>
#include <initializer_list>
#include <utility>

class GrContext;

#define WRITE_CSV 0

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

static const int outset = 100;
static const SkColor bgColor = SK_ColorWHITE;
static const int strokeWidth = 4;

static void create(SkBitmap* bm, const SkIRect& bound) {
    bm->allocN32Pixels(bound.width(), bound.height());
}

static void drawBG(SkCanvas* canvas) {
    canvas->drawColor(bgColor);
}


struct BlurTest {
    void (*addPath)(SkPath*);
    int viewLen;
    SkIRect views[9];
};

//Path Draw Procs
//Beware that paths themselves my draw differently depending on the clip.
static void draw50x50Rect(SkPath* path) {
    path->addRect(0, 0, SkIntToScalar(50), SkIntToScalar(50));
}

//Tests
static BlurTest tests[] = {
    { draw50x50Rect, 3, {
        //inner half of blur
        { 0, 0, 50, 50 },
        //blur, but no path.
        { 50 + strokeWidth/2, 50 + strokeWidth/2, 100, 100 },
        //just an edge
        { 40, strokeWidth, 60, 50 - strokeWidth },
    }},
};

/** Assumes that the ref draw was completely inside ref canvas --
    implies that everything outside is "bgColor".
    Checks that all overlap is the same and that all non-overlap on the
    ref is "bgColor".
 */
static bool compare(const SkBitmap& ref, const SkIRect& iref,
                    const SkBitmap& test, const SkIRect& itest)
{
    const int xOff = itest.fLeft - iref.fLeft;
    const int yOff = itest.fTop - iref.fTop;

    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            int refX = x + xOff;
            int refY = y + yOff;
            SkColor refColor;
            if (refX >= 0 && refX < ref.width() &&
                refY >= 0 && refY < ref.height())
            {
                refColor = ref.getColor(refX, refY);
            } else {
                refColor = bgColor;
            }
            if (refColor != testColor) {
                return false;
            }
        }
    }
    return true;
}

DEF_TEST(BlurDrawing, reporter) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(strokeWidth));

    SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5));
    for (int style = 0; style <= kLastEnum_SkBlurStyle; ++style) {
        SkBlurStyle blurStyle = static_cast<SkBlurStyle>(style);

        for (bool respectCTM : { false, true }) {
            paint.setMaskFilter(SkMaskFilter::MakeBlur(blurStyle, sigma, respectCTM));

            for (size_t test = 0; test < SK_ARRAY_COUNT(tests); ++test) {
                SkPath path;
                tests[test].addPath(&path);
                SkPath strokedPath;
                paint.getFillPath(path, &strokedPath);
                SkRect refBound = strokedPath.getBounds();
                SkIRect iref;
                refBound.roundOut(&iref);
                iref.inset(-outset, -outset);
                SkBitmap refBitmap;
                create(&refBitmap, iref);

                SkCanvas refCanvas(refBitmap);
                refCanvas.translate(SkIntToScalar(-iref.fLeft),
                                    SkIntToScalar(-iref.fTop));
                drawBG(&refCanvas);
                refCanvas.drawPath(path, paint);

                for (int view = 0; view < tests[test].viewLen; ++view) {
                    SkIRect itest = tests[test].views[view];
                    SkBitmap testBitmap;
                    create(&testBitmap, itest);

                    SkCanvas testCanvas(testBitmap);
                    testCanvas.translate(SkIntToScalar(-itest.fLeft),
                                         SkIntToScalar(-itest.fTop));
                    drawBG(&testCanvas);
                    testCanvas.drawPath(path, paint);

                    REPORTER_ASSERT(reporter,
                        compare(refBitmap, iref, testBitmap, itest));
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// Use SkBlurMask::BlurGroundTruth to blur a 'width' x 'height' solid
// white rect. Return the right half of the middle row in 'result'.
static void ground_truth_2d(int width, int height,
                            SkScalar sigma,
                            int* result, int resultCount) {
    SkMask src, dst;

    src.fBounds.setWH(width, height);
    src.fFormat = SkMask::kA8_Format;
    src.fRowBytes = src.fBounds.width();
    src.fImage = SkMask::AllocImage(src.computeTotalImageSize());

    memset(src.fImage, 0xff, src.computeTotalImageSize());

    if (!SkBlurMask::BlurGroundTruth(sigma, &dst, src, kNormal_SkBlurStyle)) {
        return;
    }

    int midX = dst.fBounds.x() + dst.fBounds.width()/2;
    int midY = dst.fBounds.y() + dst.fBounds.height()/2;
    uint8_t* bytes = dst.getAddr8(midX, midY);
    int i;
    for (i = 0; i < dst.fBounds.width()-(midX-dst.fBounds.fLeft); ++i) {
        if (i < resultCount) {
            result[i] = bytes[i];
        }
    }
    for ( ; i < resultCount; ++i) {
        result[i] = 0;
    }

    SkMask::FreeImage(src.fImage);
    SkMask::FreeImage(dst.fImage);
}

// Implement a step function that is 255 between min and max; 0 elsewhere.
static int step(int x, SkScalar min, SkScalar max) {
    if (min < x && x < max) {
        return 255;
    }
    return 0;
}

// Implement a Gaussian function with 0 mean and std.dev. of 'sigma'.
static float gaussian(int x, SkScalar sigma) {
    float k = SK_Scalar1/(sigma * sqrtf(2.0f*SK_ScalarPI));
    float exponent = -(x * x) / (2 * sigma * sigma);
    return k * expf(exponent);
}

// Perform a brute force convolution of a step function with a Gaussian.
// Return the right half in 'result'
static void brute_force_1d(SkScalar stepMin, SkScalar stepMax,
                           SkScalar gaussianSigma,
                           int* result, int resultCount) {

    int gaussianRange = SkScalarCeilToInt(10 * gaussianSigma);

    for (int i = 0; i < resultCount; ++i) {
        SkScalar sum = 0.0f;
        for (int j = -gaussianRange; j < gaussianRange; ++j) {
            sum += gaussian(j, gaussianSigma) * step(i-j, stepMin, stepMax);
        }

        result[i] = SkClampMax(SkClampPos(int(sum + 0.5f)), 255);
    }
}

static void blur_path(SkCanvas* canvas, const SkPath& path,
                      SkScalar gaussianSigma) {

    SkScalar midX = path.getBounds().centerX();
    SkScalar midY = path.getBounds().centerY();

    canvas->translate(-midX, -midY);

    SkPaint blurPaint;
    blurPaint.setColor(SK_ColorWHITE);
    blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, gaussianSigma));

    canvas->drawColor(SK_ColorBLACK);
    canvas->drawPath(path, blurPaint);
}

// Readback the blurred draw results from the canvas
static void readback(const SkBitmap& src, int* result, int resultCount) {
    SkBitmap readback;
    readback.allocN32Pixels(resultCount, 30);
    SkPixmap pm;
    readback.peekPixels(&pm);
    src.readPixels(pm, 0, 0);

    const SkPMColor* pixels = pm.addr32(0, 15);

    for (int i = 0; i < resultCount; ++i) {
        result[i] = SkColorGetR(pixels[i]);
    }
}

// Draw a blurred version of the provided path.
// Return the right half of the middle row in 'result'.
static void cpu_blur_path(const SkPath& path, SkScalar gaussianSigma,
                          int* result, int resultCount) {

    SkBitmap bitmap;
    bitmap.allocN32Pixels(resultCount, 30);
    SkCanvas canvas(bitmap);

    blur_path(&canvas, path, gaussianSigma);
    readback(bitmap, result, resultCount);
}

#if WRITE_CSV
static void write_as_csv(const char* label, SkScalar scale, int* data, int count) {
    SkDebugf("%s_%.2f,", label, scale);
    for (int i = 0; i < count-1; ++i) {
        SkDebugf("%d,", data[i]);
    }
    SkDebugf("%d\n", data[count-1]);
}
#endif

static bool match(int* first, int* second, int count, int tol) {
    int delta;
    for (int i = 0; i < count; ++i) {
        delta = first[i] - second[i];
        if (delta > tol || delta < -tol) {
            return false;
        }
    }

    return true;
}

// Test out the normal blur style with a wide range of sigmas
DEF_TEST(BlurSigmaRange, reporter) {
    static const int kSize = 100;

    // The geometry is offset a smidge to trigger:
    // https://code.google.com/p/chromium/issues/detail?id=282418
    SkPath rectPath;
    rectPath.addRect(0.3f, 0.3f, 100.3f, 100.3f);

    SkPoint polyPts[] = {
        { 0.3f, 0.3f },
        { 100.3f, 0.3f },
        { 100.3f, 100.3f },
        { 0.3f, 100.3f },
        { 2.3f, 50.3f }     // a little divet to throw off the rect special case
    };
    SkPath polyPath;
    polyPath.addPoly(polyPts, SK_ARRAY_COUNT(polyPts), true);

    int rectSpecialCaseResult[kSize];
    int generalCaseResult[kSize];
    int groundTruthResult[kSize];
    int bruteForce1DResult[kSize];

    SkScalar sigma = 10.0f;

    for (int i = 0; i < 4; ++i, sigma /= 10) {

        cpu_blur_path(rectPath, sigma, rectSpecialCaseResult, kSize);
        cpu_blur_path(polyPath, sigma, generalCaseResult, kSize);

        ground_truth_2d(100, 100, sigma, groundTruthResult, kSize);
        brute_force_1d(-50.0f, 50.0f, sigma, bruteForce1DResult, kSize);

        REPORTER_ASSERT(reporter, match(rectSpecialCaseResult, bruteForce1DResult, kSize, 5));
        REPORTER_ASSERT(reporter, match(generalCaseResult, bruteForce1DResult, kSize, 15));
        REPORTER_ASSERT(reporter, match(groundTruthResult, bruteForce1DResult, kSize, 1));

#if WRITE_CSV
        write_as_csv("RectSpecialCase", sigma, rectSpecialCaseResult, kSize);
        write_as_csv("GeneralCase", sigma, generalCaseResult, kSize);
        write_as_csv("GPU", sigma, gpuResult, kSize);
        write_as_csv("GroundTruth2D", sigma, groundTruthResult, kSize);
        write_as_csv("BruteForce1D", sigma, bruteForce1DResult, kSize);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

static void test_blurDrawLooper(skiatest::Reporter* reporter, SkScalar sigma, SkBlurStyle style) {
    if (kNormal_SkBlurStyle != style) {
        return; // blurdrawlooper only supports normal
    }

    const SkColor color = 0xFF335577;
    const SkScalar dx = 10;
    const SkScalar dy = -5;
    sk_sp<SkDrawLooper> lp(SkBlurDrawLooper::Make(color, sigma, dx, dy));
    const bool expectSuccess = sigma > 0;

    if (nullptr == lp) {
        REPORTER_ASSERT(reporter, sigma <= 0);
    } else {
        SkDrawLooper::BlurShadowRec rec;
        bool success = lp->asABlurShadow(&rec);
        REPORTER_ASSERT(reporter, success == expectSuccess);
        if (success) {
            REPORTER_ASSERT(reporter, rec.fSigma == sigma);
            REPORTER_ASSERT(reporter, rec.fOffset.x() == dx);
            REPORTER_ASSERT(reporter, rec.fOffset.y() == dy);
            REPORTER_ASSERT(reporter, rec.fColor == color);
            REPORTER_ASSERT(reporter, rec.fStyle == style);
        }
    }
}

static void test_looper(skiatest::Reporter* reporter, sk_sp<SkDrawLooper> lp, SkScalar sigma,
                        SkBlurStyle style, bool expectSuccess) {
    SkDrawLooper::BlurShadowRec rec;
    bool success = lp->asABlurShadow(&rec);
    REPORTER_ASSERT(reporter, success == expectSuccess);
    if (success != expectSuccess) {
        lp->asABlurShadow(&rec);
    }
    if (success) {
        REPORTER_ASSERT(reporter, rec.fSigma == sigma);
        REPORTER_ASSERT(reporter, rec.fStyle == style);
    }
}

static void make_noop_layer(SkLayerDrawLooper::Builder* builder) {
    SkLayerDrawLooper::LayerInfo info;

    info.fPaintBits = 0;
    info.fColorMode = SkBlendMode::kDst;
    builder->addLayer(info);
}

static void make_blur_layer(SkLayerDrawLooper::Builder* builder, sk_sp<SkMaskFilter> mf) {
    SkLayerDrawLooper::LayerInfo info;

    info.fPaintBits = SkLayerDrawLooper::kMaskFilter_Bit;
    info.fColorMode = SkBlendMode::kSrc;
    SkPaint* paint = builder->addLayer(info);
    paint->setMaskFilter(std::move(mf));
}

static void test_layerDrawLooper(skiatest::Reporter* reporter, sk_sp<SkMaskFilter> mf,
                                 SkScalar sigma, SkBlurStyle style, bool expectSuccess) {

    SkLayerDrawLooper::LayerInfo info;
    SkLayerDrawLooper::Builder builder;

    // 1 layer is too few
    make_noop_layer(&builder);
    test_looper(reporter, builder.detach(), sigma, style, false);

    // 2 layers is good, but need blur
    make_noop_layer(&builder);
    make_noop_layer(&builder);
    test_looper(reporter, builder.detach(), sigma, style, false);

    // 2 layers is just right
    make_noop_layer(&builder);
    make_blur_layer(&builder, mf);
    test_looper(reporter, builder.detach(), sigma, style, expectSuccess);

    // 3 layers is too many
    make_noop_layer(&builder);
    make_blur_layer(&builder, mf);
    make_noop_layer(&builder);
    test_looper(reporter, builder.detach(), sigma, style, false);
}

DEF_TEST(BlurAsABlur, reporter) {
    const SkBlurStyle styles[] = {
        kNormal_SkBlurStyle, kSolid_SkBlurStyle, kOuter_SkBlurStyle, kInner_SkBlurStyle
    };
    const SkScalar sigmas[] = {
        // values <= 0 should not success for a blur
        -1, 0, 0.5f, 2
    };

    // Test asABlur for SkBlurMaskFilter
    //
    for (size_t i = 0; i < SK_ARRAY_COUNT(styles); ++i) {
        const SkBlurStyle style = styles[i];
        for (size_t j = 0; j < SK_ARRAY_COUNT(sigmas); ++j) {
            const SkScalar sigma = sigmas[j];
            for (bool respectCTM : { false, true }) {
                sk_sp<SkMaskFilter> mf(SkMaskFilter::MakeBlur(style, sigma, respectCTM));
                if (nullptr == mf.get()) {
                    REPORTER_ASSERT(reporter, sigma <= 0);
                } else {
                    REPORTER_ASSERT(reporter, sigma > 0);
                    SkMaskFilterBase::BlurRec rec;
                    bool success = as_MFB(mf)->asABlur(&rec);
                    if (respectCTM) {
                        REPORTER_ASSERT(reporter, success);
                        REPORTER_ASSERT(reporter, rec.fSigma == sigma);
                        REPORTER_ASSERT(reporter, rec.fStyle == style);
                    } else {
                        REPORTER_ASSERT(reporter, !success);
                    }
                    test_layerDrawLooper(reporter, std::move(mf), sigma, style, success);
                }
                test_blurDrawLooper(reporter, sigma, style);
            }
        }
    }

    // Test asABlur for SkEmbossMaskFilter -- should never succeed
    //
    {
        SkEmbossMaskFilter::Light light = {
            { 1, 1, 1 }, 0, 127, 127
        };
        for (size_t j = 0; j < SK_ARRAY_COUNT(sigmas); ++j) {
            const SkScalar sigma = sigmas[j];
            auto mf(SkEmbossMaskFilter::Make(sigma, light));
            if (mf) {
                SkMaskFilterBase::BlurRec rec;
                bool success = as_MFB(mf)->asABlur(&rec);
                REPORTER_ASSERT(reporter, !success);
            }
        }
    }
}

// This exercises the problem discovered in crbug.com/570232. The return value from
// SkBlurMask::BoxBlur wasn't being checked in SkBlurMaskFilter.cpp::GrRRectBlurEffect::Create
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SmallBoxBlurBug, reporter, ctxInfo) {

    SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
    auto surface(SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeXYWH(10, 10, 100, 100);
    SkRRect rr = SkRRect::MakeRectXY(r, 10, 10);

    SkPaint p;
    p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.01f));

    canvas->drawRRect(rr, p);
}

DEF_TEST(BlurredRRectNinePatchComputation, reporter) {
    const SkRect r = SkRect::MakeXYWH(10, 10, 100, 100);
    static const SkScalar kBlurRad = 3.0f;

    bool ninePatchable;
    SkRRect rrectToDraw;
    SkISize size;
    SkScalar rectXs[kSkBlurRRectMaxDivisions],
             rectYs[kSkBlurRRectMaxDivisions];
    SkScalar texXs[kSkBlurRRectMaxDivisions],
             texYs[kSkBlurRRectMaxDivisions];
    int numX, numY;
    uint32_t skipMask;

    // not nine-patchable
    {
        SkVector radii[4] = { { 100, 100 }, { 0, 0 }, { 100, 100 }, { 0, 0 } };

        SkRRect rr;
        rr.setRectRadii(r, radii);

        ninePatchable = SkComputeBlurredRRectParams(rr, rr, SkRect::MakeEmpty(),
                                                    kBlurRad, kBlurRad,
                                                    &rrectToDraw, &size,
                                                    rectXs, rectYs, texXs, texYs,
                                                    &numX, &numY, &skipMask);
        REPORTER_ASSERT(reporter, !ninePatchable);
    }

    // simple circular
    {
        static const SkScalar kCornerRad = 10.0f;
        SkRRect rr;
        rr.setRectXY(r, kCornerRad, kCornerRad);

        ninePatchable = SkComputeBlurredRRectParams(rr, rr, SkRect::MakeEmpty(),
                                                    kBlurRad, kBlurRad,
                                                    &rrectToDraw, &size,
                                                    rectXs, rectYs, texXs, texYs,
                                                    &numX, &numY, &skipMask);

        static const SkScalar kAns = 12.0f * kBlurRad + 2.0f * kCornerRad + 1.0f;
        REPORTER_ASSERT(reporter, ninePatchable);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkIntToScalar(size.fWidth), kAns));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkIntToScalar(size.fHeight), kAns));
        REPORTER_ASSERT(reporter, 4 == numX && 4 == numY);
        REPORTER_ASSERT(reporter, !skipMask);
    }

    // simple elliptical
    {
        static const SkScalar kXCornerRad = 2.0f;
        static const SkScalar kYCornerRad = 10.0f;
        SkRRect rr;
        rr.setRectXY(r, kXCornerRad, kYCornerRad);

        ninePatchable = SkComputeBlurredRRectParams(rr, rr, SkRect::MakeEmpty(),
                                                    kBlurRad, kBlurRad,
                                                    &rrectToDraw, &size,
                                                    rectXs, rectYs, texXs, texYs,
                                                    &numX, &numY, &skipMask);

        static const SkScalar kXAns = 12.0f * kBlurRad + 2.0f * kXCornerRad + 1.0f;
        static const SkScalar kYAns = 12.0f * kBlurRad + 2.0f * kYCornerRad + 1.0f;

        REPORTER_ASSERT(reporter, ninePatchable);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkIntToScalar(size.fWidth), kXAns));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SkIntToScalar(size.fHeight), kYAns));
        REPORTER_ASSERT(reporter, 4 == numX && 4 == numY);
        REPORTER_ASSERT(reporter, !skipMask);
    }

    // test-out occlusion
    {
        static const SkScalar kCornerRad = 10.0f;
        SkRRect rr;
        rr.setRectXY(r, kCornerRad, kCornerRad);

        // The rectXs & rectYs should be { 1, 29, 91, 119 }. Add two more points around each.
        SkScalar testLocs[] = {
             -18.0f, -9.0f,
               1.0f,
               9.0f, 18.0f,
              29.0f,
              39.0f, 49.0f,
              91.0f,
             109.0f, 118.0f,
             119.0f,
             139.0f, 149.0f
        };

        for (int minY = 0; minY < (int)SK_ARRAY_COUNT(testLocs); ++minY) {
            for (int maxY = minY+1; maxY < (int)SK_ARRAY_COUNT(testLocs); ++maxY) {
                for (int minX = 0; minX < (int)SK_ARRAY_COUNT(testLocs); ++minX) {
                    for (int maxX = minX+1; maxX < (int)SK_ARRAY_COUNT(testLocs); ++maxX) {
                        SkRect occluder = SkRect::MakeLTRB(testLocs[minX], testLocs[minY],
                                                           testLocs[maxX], testLocs[maxY]);
                        if (occluder.isEmpty()) {
                            continue;
                        }

                        ninePatchable = SkComputeBlurredRRectParams(rr, rr, occluder,
                                                                    kBlurRad, kBlurRad,
                                                                    &rrectToDraw, &size,
                                                                    rectXs, rectYs, texXs, texYs,
                                                                    &numX, &numY, &skipMask);

                        static const SkScalar kAns = 12.0f * kBlurRad + 2.0f * kCornerRad + 1.0f;
                        REPORTER_ASSERT(reporter, ninePatchable);
                        REPORTER_ASSERT(reporter,
                                            SkScalarNearlyEqual(SkIntToScalar(size.fWidth), kAns));
                        REPORTER_ASSERT(reporter,
                                            SkScalarNearlyEqual(SkIntToScalar(size.fHeight), kAns));

                        int checkBit = 0x1;
                        for (int y = 0; y < numY-1; ++y) {
                            for (int x = 0; x < numX-1; ++x) {
                                SkRect cell = SkRect::MakeLTRB(rectXs[x], rectYs[y],
                                                               rectXs[x+1], rectYs[y+1]);
                                REPORTER_ASSERT(reporter,
                                                    SkToBool(skipMask & checkBit) ==
                                                    (cell.isEmpty() || occluder.contains(cell)));

                                REPORTER_ASSERT(reporter, texXs[x] >= 0 &&
                                                          texXs[x] <= size.fWidth);
                                REPORTER_ASSERT(reporter, texYs[y] >= 0 &&
                                                          texXs[y] <= size.fHeight);

                                checkBit <<= 1;
                            }
                        }
                    }
                }
            }
        }


    }

}

// https://crbugs.com/787712
DEF_TEST(EmbossPerlinCrash, reporter) {
    SkPaint p;

    static constexpr SkEmbossMaskFilter::Light light = {
        { 1, 1, 1 }, 0, 127, 127
    };
    p.setMaskFilter(SkEmbossMaskFilter::Make(1, light));
    p.setShader(SkPerlinNoiseShader::MakeFractalNoise(1.0f, 1.0f, 2, 0.0f));

    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);
    surface->getCanvas()->drawPaint(p);
}

///////////////////////////////////////////////////////////////////////////////////////////

DEF_TEST(BlurZeroSigma, reporter) {
    auto surf = SkSurface::MakeRasterN32Premul(20, 20);
    SkPaint paint;
    paint.setAntiAlias(true);

    const SkIRect ir = { 5, 5, 15, 15 };
    const SkRect r = SkRect::Make(ir);

    const SkScalar sigmas[] = { 0, SkBits2Float(1) };
    // if sigma is zero (or nearly so), we need to draw correctly (unblurred) and not crash
    // or assert.
    for (auto sigma : sigmas) {
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        surf->getCanvas()->drawRect(r, paint);

        ToolUtils::PixelIter iter(surf.get());
        SkIPoint  loc;
        while (const SkPMColor* p = (const SkPMColor*)iter.next(&loc)) {
            if (ir.contains(loc.fX, loc.fY)) {
                // inside the rect we draw (opaque black)
                REPORTER_ASSERT(reporter, *p == SkPackARGB32(0xFF, 0, 0, 0));
            } else {
                // outside the rect we didn't draw at all, no blurred edges
                REPORTER_ASSERT(reporter, *p == 0);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BlurMaskBiggerThanDest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    SkImageInfo ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> dst(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii));
    if (!dst) {
        ERRORF(reporter, "Could not create surface for test.");
        return;
    }

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 3));

    SkCanvas* canvas = dst->getCanvas();

    canvas->clear(SK_ColorBLACK);
    canvas->drawCircle(SkPoint::Make(16, 16), 8, p);

    SkBitmap readback;
    SkAssertResult(readback.tryAllocPixels(ii));

    canvas->readPixels(readback, 0, 0);
    REPORTER_ASSERT(reporter, SkColorGetR(readback.getColor(15, 15)) > 128);
    REPORTER_ASSERT(reporter, SkColorGetG(readback.getColor(15, 15)) == 0);
    REPORTER_ASSERT(reporter, SkColorGetB(readback.getColor(15, 15)) == 0);
    REPORTER_ASSERT(reporter, readback.getColor(31, 31) == SK_ColorBLACK);
}

DEF_TEST(zero_blur, reporter) {
    SkBitmap alpha, bitmap;

    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3));
    SkIPoint offset;
    bitmap.extractAlpha(&alpha, &paint, nullptr, &offset);
}

