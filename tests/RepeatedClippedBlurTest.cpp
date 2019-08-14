/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "src/gpu/GrContextPriv.h"
#include "tests/Test.h"

// This is the repro of a CastOS memory regression bug (b/138674523).
// The test simply keeps calling SkImage::makeWithFilter (with a blur image filter) while
// shrinking the clip.
// When explicit resource allocation was enabled the last (re-expanded) image in the
// blur creation process became exact.
// This meant that its backing texture could no longer be reused.
// In CastOS' case (and, presumably, Linux desktop) they were only using Ganesh for
// 2D canvas and compositor image filtering. In this case Chrome doesn't regularly purge
// the cache. This would result in Ganesh quickly running up to its max cache limit.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RepeatedClippedBlurTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceCache* cache = context->priv().getResourceCache();

    const SkImageInfo ii = SkImageInfo::Make(1024, 600, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> dst(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii));
    if (!dst) {
        ERRORF(reporter, "Could not create surfaces for repeated clipped blur test.");
        return;
    }

    SkCanvas* dstCanvas = dst->getCanvas();

    sk_sp<SkImage> bigImg;

    // Create the initial big image (this corresponds to the album artwork - which is larger
    // than the screen)
    {
        SkImageInfo srcImageII = SkImageInfo::Make(1280, 1280, kRGBA_8888_SkColorType,
                                                   kPremul_SkAlphaType);

        // Make a red ring around a field of green. When rendered the blurred red ring
        // should still be visible on all sides of the dest image.
        SkBitmap bm;
        bm.allocPixels(srcImageII);
        bm.eraseColor(SK_ColorRED);
        bm.eraseArea(SkIRect::MakeXYWH(1, 2, 1277, 1274), SK_ColorGREEN);

        sk_sp<SkImage> rasterImg = SkImage::MakeFromBitmap(bm);
        bigImg = rasterImg->makeTextureImage(context);
    }

    sk_sp<SkImage> smImg;

    // Shrink the album artwork down to the screen's size
    {
        SkImageInfo screenII = SkImageInfo::Make(1024, 600, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);

        sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                                         screenII, 1, kTopLeft_GrSurfaceOrigin,
                                                         nullptr);
        SkCanvas* c = s->getCanvas();

        c->drawImageRect(bigImg, SkRect::MakeWH(1024, 600), nullptr);

        smImg = s->makeImageSnapshot();
    }

    // flush here just to clear the playing field
    context->flush();

    size_t beforeBytes = cache->getResourceBytes();

    // Now draw the screen-sized image, blurred, multiple times with a shrinking clip.
    // This simulates the swipe away where the screen-sized album artwork is moved off
    // screen.
    // Note that the blur has to big enough to kick the blur code into the decimate then
    // re-expand case.
    const SkIRect subset = SkIRect::MakeWH(1024, 600);
    SkIRect clip = SkIRect::MakeWH(1024, 600);

    for (int i = 0; i < 30; ++i) {
        dstCanvas->clear(SK_ColorBLUE);

        sk_sp<SkImageFilter> blur = SkImageFilters::Blur(20, 20, nullptr);

        SkIRect outSubset;
        SkIPoint offset;
        sk_sp<SkImage> filteredImg = smImg->makeWithFilter(context, blur.get(), subset, clip,
                                                           &outSubset, &offset);

        SkRect dstRect = SkRect::MakeXYWH(offset.fX, offset.fY,
                                          outSubset.width(), outSubset.height());
        dstCanvas->drawImageRect(filteredImg, outSubset, dstRect, nullptr);

        // Flush here to mimic Chrome's SkiaHelper::ApplyImageFilter
        context->flush();

        clip.fRight -= 16;
    }

    size_t afterBytes = cache->getResourceBytes();

    // When the bug manifests the resource cache will accumulate ~80MB. If texture recycling
    // is working as expected the cache size will level off at ~20MB.
    REPORTER_ASSERT(reporter, afterBytes < beforeBytes + 20000000);
}
