/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file
*/

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "Test.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "SkGr.h"

class TestingSpecialSurfaceAccess {
public:
    static const SkIRect& Subset(const SkSpecialSurface* surf) {
        return surf->subset();
    }
};

// Both 'kSmallerSize' and 'kFullSize' need to be a non-power-of-2 to exercise
// the gpu's loose fit behavior
static const int kSmallerSize = 10;
static const int kPad = 5;
static const int kFullSize = kSmallerSize + 2 * kPad;

// Exercise the public API of SkSpecialSurface (e.g., getCanvas, newImageSnapshot)
static void test_surface(const sk_sp<SkSpecialSurface>& surf,
                         skiatest::Reporter* reporter,
                         int offset) {

    const SkIRect surfSubset = TestingSpecialSurfaceAccess::Subset(surf.get());
    REPORTER_ASSERT(reporter, offset == surfSubset.fLeft);
    REPORTER_ASSERT(reporter, offset == surfSubset.fTop);
    REPORTER_ASSERT(reporter, kSmallerSize == surfSubset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == surfSubset.height());

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT_RELEASE(canvas);

    canvas->clear(SK_ColorRED);

    sk_sp<SkSpecialImage> img(surf->makeImageSnapshot());
    REPORTER_ASSERT(reporter, img);

    const SkIRect imgSubset = img->subset();
    REPORTER_ASSERT(reporter, surfSubset == imgSubset);

    // the canvas was invalidated by the newImageSnapshot call
    REPORTER_ASSERT(reporter, !surf->getCanvas());
}

DEF_TEST(SpecialSurface_Raster, reporter) {

    SkImageInfo info = SkImageInfo::MakeN32(kSmallerSize, kSmallerSize, kOpaque_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(SkSpecialSurface::MakeRaster(info));

    test_surface(surf, reporter, 0);
}

DEF_TEST(SpecialSurface_Raster2, reporter) {

    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);

    const SkIRect subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialSurface> surf(SkSpecialSurface::MakeFromBitmap(subset, bm));

    test_surface(surf, reporter, kPad);

    // TODO: check that the clear didn't escape the active region
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialSurface_Gpu1, reporter, ctxInfo) {
    for (auto config : { kRGBA_8888_GrPixelConfig, kRGBA_1010102_GrPixelConfig }) {
        if (!ctxInfo.grContext()->contextPriv().caps()->isConfigRenderable(config)) {
            continue;
        }
        sk_sp<SkSpecialSurface> surf(SkSpecialSurface::MakeRenderTarget(ctxInfo.grContext(),
                                                                        kSmallerSize, kSmallerSize,
                                                                        config, nullptr));
        test_surface(surf, reporter, 0);
    }
}
