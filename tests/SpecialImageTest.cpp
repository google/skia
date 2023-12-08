/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/ganesh/GrColorInfo.h" // IWYU pragma: keep
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <utility>

class GrRecordingContext;
struct GrContextOptions;

// This test creates backing resources exactly sized to [kFullSize x kFullSize].
// It then wraps them in an SkSpecialImage with only the center (red) region being active.
// It then draws the SkSpecialImage to a full sized (all blue) canvas and checks that none
// of the inactive (green) region leaked out.

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

// Create a bitmap with red in the center and green around it
static SkBitmap create_bm() {
    SkImageInfo ii = SkImageInfo::Make(kFullSize, kFullSize, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas temp(bm);

    temp.clear(SK_ColorGREEN);
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(false);

    temp.drawRect(SkRect::MakeXYWH(SkIntToScalar(kPad), SkIntToScalar(kPad),
                                   SkIntToScalar(kSmallerSize), SkIntToScalar(kSmallerSize)),
                  p);

    bm.setImmutable();
    return bm;
}

// Basic test of the SkSpecialImage public API (e.g., peekTexture, peekPixels & draw)
static void test_image(const sk_sp<SkSpecialImage>& img, skiatest::Reporter* reporter,
                       GrRecordingContext* rContext, bool isGPUBacked) {
    const SkIRect subset = img->subset();
    REPORTER_ASSERT(reporter, kPad == subset.left());
    REPORTER_ASSERT(reporter, kPad == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    // Test that isTextureBacked reports the correct backing type
    REPORTER_ASSERT(reporter, isGPUBacked == img->isGaneshBacked());
    REPORTER_ASSERT(reporter, !img->isGraphiteBacked());

    //--------------
    // Test view - only succeeds if it's Ganesh backed
    if (rContext) {
        GrSurfaceProxyView view = SkSpecialImages::AsView(rContext, img);
        REPORTER_ASSERT(reporter, SkToBool(view.asTextureProxy()) == isGPUBacked);
    }

    //--------------
    // Test AsBitmap - this only works for raster-backed special images
    if (!img->isGaneshBacked() && !img->isGraphiteBacked()) {
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, SkSpecialImages::AsBitmap(img.get(), &bitmap));
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.width());
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.height());
    } else {
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, !SkSpecialImages::AsBitmap(img.get(), &bitmap));
    }

    //--------------
    // Test that draw restricts itself to the subset
    SkImageInfo imageInfo = SkImageInfo::Make(SkISize::Make(kFullSize, kFullSize),
                                              kN32_SkColorType,
                                              kPremul_SkAlphaType,
                                              sk_ref_sp(img->getColorSpace()));
    sk_sp<SkSurface> surf = isGPUBacked
            ? SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kNo, imageInfo)
            : SkSurfaces::Raster(imageInfo, {});

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorBLUE);
    img->draw(canvas, SkIntToScalar(kPad), SkIntToScalar(kPad));

    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, false);

    bool result = canvas->readPixels(bm.info(), bm.getPixels(), bm.rowBytes(), 0, 0);
    SkASSERT_RELEASE(result);

    // Only the center (red) portion should've been drawn into the canvas
    REPORTER_ASSERT(reporter, SK_ColorBLUE == bm.getColor(kPad-1, kPad-1));
    REPORTER_ASSERT(reporter, SK_ColorRED  == bm.getColor(kPad, kPad));
    REPORTER_ASSERT(reporter, SK_ColorRED  == bm.getColor(kSmallerSize+kPad-1,
                                                          kSmallerSize+kPad-1));
    REPORTER_ASSERT(reporter, SK_ColorBLUE == bm.getColor(kSmallerSize+kPad,
                                                          kSmallerSize+kPad));
}

DEF_TEST(SpecialImage_Raster, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImages::MakeFromRaster(
            SkIRect::MakeWH(kFullSize, kFullSize), bm, SkSurfaceProps()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(
                SkSpecialImages::MakeFromRaster(subset, bm, SkSurfaceProps()));
        test_image(subSImg1, reporter, nullptr, false);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, nullptr, false);
    }
}

static void test_specialimage_image(skiatest::Reporter* reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkImage> fullImage(bm.asImage());

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImages::MakeFromRaster(
            SkIRect::MakeWH(kFullSize, kFullSize), fullImage, SkSurfaceProps()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(
                SkSpecialImages::MakeFromRaster(subset, fullImage, SkSurfaceProps()));
        test_image(subSImg1, reporter, nullptr, false);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, nullptr, false);
    }
}

DEF_TEST(SpecialImage_Image_Legacy, reporter) {
    test_specialimage_image(reporter);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SpecialImage_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    SkBitmap bm = create_bm();
    auto [view, ct] = GrMakeUncachedBitmapProxyView(context, bm);
    if (!view) {
        return;
    }

    sk_sp<SkSpecialImage> fullSImg =
            SkSpecialImages::MakeDeferredFromGpu(context,
                                                 SkIRect::MakeWH(kFullSize, kFullSize),
                                                 kNeedNewImageUniqueID_SpecialImage,
                                                 view,
                                                 {ct, kPremul_SkAlphaType, nullptr},
                                                 SkSurfaceProps());

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1 =
                SkSpecialImages::MakeDeferredFromGpu(context,
                                                     subset,
                                                     kNeedNewImageUniqueID_SpecialImage,
                                                     std::move(view),
                                                     {ct, kPremul_SkAlphaType, nullptr},
                                                     SkSurfaceProps());
        test_image(subSImg1, reporter, context, true);
    }

    {
        sk_sp<SkSpecialImage> subSImg2 = fullSImg->makeSubset(subset);
        test_image(subSImg2, reporter, context, true);
    }
}
