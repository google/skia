/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"

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
    REPORTER_ASSERT(reporter, isGPUBacked == img->isTextureBacked());

    //--------------
    // Test view - as long as there is a context this should succeed
    if (rContext) {
        GrSurfaceProxyView view = img->view(rContext);
        REPORTER_ASSERT(reporter, view.asTextureProxy());
    }

    //--------------
    // Test getROPixels - this only works for raster-backed special images
    if (!img->isTextureBacked()) {
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, img->getROPixels(&bitmap));
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.width());
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.height());
    }

    //--------------
    // Test that draw restricts itself to the subset
    sk_sp<SkSpecialSurface> surf(img->makeSurface(kN32_SkColorType, img->getColorSpace(),
                                                  SkISize::Make(kFullSize, kFullSize),
                                                  kPremul_SkAlphaType));

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

    //--------------
    // Test that asImage & makeTightSurface return appropriately sized objects
    // of the correct backing type
    SkIRect newSubset = SkIRect::MakeWH(subset.width(), subset.height());
    {
        sk_sp<SkImage> tightImg(img->asImage(&newSubset));

        REPORTER_ASSERT(reporter, tightImg->width() == subset.width());
        REPORTER_ASSERT(reporter, tightImg->height() == subset.height());
        REPORTER_ASSERT(reporter, isGPUBacked == tightImg->isTextureBacked());
        SkPixmap tmpPixmap;
        REPORTER_ASSERT(reporter, isGPUBacked != !!tightImg->peekPixels(&tmpPixmap));
    }
    {
        sk_sp<SkSurface> tightSurf(img->makeTightSurface(kN32_SkColorType, img->getColorSpace(),
                                                         subset.size()));

        REPORTER_ASSERT(reporter, tightSurf->width() == subset.width());
        REPORTER_ASSERT(reporter, tightSurf->height() == subset.height());
        GrBackendTexture backendTex = tightSurf->getBackendTexture(
                                                    SkSurface::kDiscardWrite_BackendHandleAccess);
        REPORTER_ASSERT(reporter, isGPUBacked == backendTex.isValid());
        SkPixmap tmpPixmap;
        REPORTER_ASSERT(reporter, isGPUBacked != !!tightSurf->peekPixels(&tmpPixmap));
    }
}

DEF_TEST(SpecialImage_Raster, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromRaster(
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            bm));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromRaster(subset, bm));
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

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromImage(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            fullImage));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromImage(nullptr, subset, fullImage));
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_Gpu, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    SkBitmap bm = create_bm();
    GrBitmapTextureMaker maker(context, bm, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    auto view = maker.view(GrMipmapped::kNo);
    if (!view.proxy()) {
        return;
    }

    sk_sp<SkSpecialImage> fullSImg(
            SkSpecialImage::MakeDeferredFromGpu(context,
                                                SkIRect::MakeWH(kFullSize, kFullSize),
                                                kNeedNewImageUniqueID_SpecialImage,
                                                view,
                                                maker.colorType(),
                                                nullptr));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeDeferredFromGpu(
                context,
                subset,
                kNeedNewImageUniqueID_SpecialImage,
                std::move(view),
                maker.colorType(),
                nullptr));
        test_image(subSImg1, reporter, context, true);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, context, true);
    }
}
