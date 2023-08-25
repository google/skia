/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTemplates.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"

#include <initializer_list>
#include <vector>

struct GrContextOptions;
struct Results { int diffs, diffs_0x00, diffs_0xff, diffs_by_1; };

static bool acceptable(const Results& r) {
#if 0
    SkDebugf("%d diffs, %d at 0x00, %d at 0xff, %d off by 1, all out of 65536\n",
             r.diffs, r.diffs_0x00, r.diffs_0xff, r.diffs_by_1);
#endif
    return r.diffs_by_1 == r.diffs   // never off by more than 1
        && r.diffs_0x00 == 0         // transparent must stay transparent
        && r.diffs_0xff == 0;        // opaque must stay opaque
}

template <typename Fn>
static Results test(Fn&& multiply) {
    Results r = { 0,0,0,0 };
    for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
        int p = multiply(x, y),
            ideal = (x*y+127)/255;
        if (p != ideal) {
            r.diffs++;
            if (x == 0x00 || y == 0x00) { r.diffs_0x00++; }
            if (x == 0xff || y == 0xff) { r.diffs_0xff++; }
            if (SkTAbs(ideal - p) == 1) { r.diffs_by_1++; }
        }
    }}
    return r;
}

DEF_TEST(Blend_byte_multiply, r) {
    // These are all temptingly close but fundamentally broken.
    int (*broken[])(int, int) = {
        [](int x, int y) { return (x*y)>>8; },
        [](int x, int y) { return (x*y+128)>>8; },
        [](int x, int y) { y += y>>7; return (x*y)>>8; },
    };
    for (auto multiply : broken) { REPORTER_ASSERT(r, !acceptable(test(multiply))); }

    // These are fine to use, but not perfect.
    int (*fine[])(int, int) = {
        [](int x, int y) { return (x*y+x)>>8; },
        [](int x, int y) { return (x*y+y)>>8; },
        [](int x, int y) { return (x*y+255)>>8; },
        [](int x, int y) { y += y>>7; return (x*y+128)>>8; },
    };
    for (auto multiply : fine) { REPORTER_ASSERT(r, acceptable(test(multiply))); }

    // These are pefect.
    int (*perfect[])(int, int) = {
        [](int x, int y) { return (x*y+127)/255; },  // Duh.
        [](int x, int y) { int p = (x*y+128); return (p+(p>>8))>>8; },
        [](int x, int y) { return ((x*y+128)*257)>>16; },
    };
    for (auto multiply : perfect) { REPORTER_ASSERT(r, test(multiply).diffs == 0); }
}

// Tests blending to a surface with no texture available.
DEF_GANESH_TEST_FOR_GL_CONTEXT(ES2BlendWithNoTexture,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    static constexpr SkISize kDimensions{10, 10};
    const SkColorType kColorType = kRGBA_8888_SkColorType;

    // Build our test cases:
    struct RectAndSamplePoint {
        SkRect rect;
        SkIPoint outPoint;
        SkIPoint inPoint;
    } allRectsAndPoints[3] = {
            {SkRect::MakeXYWH(0, 0, 5, 5), SkIPoint::Make(7, 7), SkIPoint::Make(2, 2)},
            {SkRect::MakeXYWH(2, 2, 5, 5), SkIPoint::Make(1, 1), SkIPoint::Make(4, 4)},
            {SkRect::MakeXYWH(5, 5, 5, 5), SkIPoint::Make(2, 2), SkIPoint::Make(7, 7)},
    };

    struct TestCase {
        RectAndSamplePoint fRectAndPoints;
        SkRect             fClip;
        int                fSampleCnt;
        GrSurfaceOrigin    fOrigin;
    };
    std::vector<TestCase> testCases;

    for (auto origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        for (int sampleCnt : {1, 4}) {
            for (auto rectAndPoints : allRectsAndPoints) {
                for (auto clip : {SkRect::MakeXYWH(0, 0, 10, 10), SkRect::MakeXYWH(1, 1, 8, 8)}) {
                    testCases.push_back({rectAndPoints, clip, sampleCnt, origin});
                }
            }
        }
    }

    // Run each test case:
    for (auto testCase : testCases) {
        int sampleCnt = testCase.fSampleCnt;
        SkRect paintRect = testCase.fRectAndPoints.rect;
        SkIPoint outPoint = testCase.fRectAndPoints.outPoint;
        SkIPoint inPoint = testCase.fRectAndPoints.inPoint;
        GrSurfaceOrigin origin = testCase.fOrigin;

        // BGRA forces a framebuffer blit on ES2.
        sk_sp<SkSurface> surface = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                               kDimensions,
                                                                               origin,
                                                                               sampleCnt,
                                                                               kColorType);

        if (!surface && sampleCnt > 1) {
            // Some platforms don't support MSAA.
            continue;
        }
        REPORTER_ASSERT(reporter, !!surface);

        // Fill our canvas with 0xFFFF80
        SkCanvas* canvas = surface->getCanvas();
        canvas->clipRect(testCase.fClip, false);
        SkPaint black_paint;
        black_paint.setColor(SkColorSetRGB(0xFF, 0xFF, 0x80));
        canvas->drawRect(SkRect::Make(kDimensions), black_paint);

        // Blend 2x2 pixels at 5,5 with 0x80FFFF. Use multiply blend mode as this will trigger
        // a copy of the destination.
        SkPaint white_paint;
        white_paint.setColor(SkColorSetRGB(0x80, 0xFF, 0xFF));
        white_paint.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawRect(paintRect, white_paint);

        // Read the result into a bitmap.
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, bitmap.tryAllocPixels(SkImageInfo::Make(kDimensions, kColorType,
                                                                          kPremul_SkAlphaType)));
        REPORTER_ASSERT(
                reporter,
                surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), 0, 0));

        // Check the in/out pixels.
        REPORTER_ASSERT(reporter, bitmap.getColor(outPoint.x(), outPoint.y()) ==
                                          SkColorSetRGB(0xFF, 0xFF, 0x80));
        REPORTER_ASSERT(reporter, bitmap.getColor(inPoint.x(), inPoint.y()) ==
                                          SkColorSetRGB(0x80, 0xFF, 0x80));
    }
}

// Test that dst reads when large coordinates read the correct pixels.
// When we use half-width floats for dst read coordinates, we can end up reading the wrong pixel
// from dst and consequently writing the wrong blended color (skbug.com/14347).
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BlendRequiringDstReadWithLargeCoordinates,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    static constexpr SkColorType kColorType = kRGBA_8888_SkColorType;

    GrDirectContext* context = contextInfo.directContext();
    SkImageInfo imageInfo =
            SkImageInfo::Make(SkISize::Make(1200, 1), kColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, imageInfo);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);

    // Draw a red rectangle at x=1100.
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawIRect(SkIRect::MakeXYWH(1100, 0, 5, 1), paint);

    // Draw a rectangle with a blend mode that requires a dst read, over the drawn red rectangle.
    SkPaint blendPaint;
    blendPaint.setBlendMode(SkBlendMode::kSoftLight);
    canvas->drawIRect(SkIRect::MakeXYWH(1090, 0, 20, 1), blendPaint);

    // Check the pixels at the edge of the left intersection between the two rectangles.
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter,
                    bitmap.tryAllocPixels(SkImageInfo::Make(
                            SkISize::Make(2, 1), kColorType, kPremul_SkAlphaType)));
    REPORTER_ASSERT(
            reporter,
            surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), 1099, 0));

    REPORTER_ASSERT(reporter, bitmap.getColor(0, 0) == SK_ColorBLACK);
    REPORTER_ASSERT(reporter, bitmap.getColor(1, 0) == SK_ColorRED);
}
