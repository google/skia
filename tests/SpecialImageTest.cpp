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
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "tests/Test.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"


// This test creates backing resources exactly sized to [kFullSize x kFullSize].
// It then wraps them in an SkSpecialImage with only the center (red) region being active.
// It then draws the SkSpecialImage to a full sized (all blue) canvas and checks that none
// of the inactive (green) region leaked out.

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

// Create a bitmap with red in the center and green around it
static SkBitmap create_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);

    SkCanvas temp(bm);

    temp.clear(SK_ColorGREEN);
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(false);

    temp.drawRect(SkRect::MakeXYWH(SkIntToScalar(kPad), SkIntToScalar(kPad),
                                   SkIntToScalar(kSmallerSize), SkIntToScalar(kSmallerSize)),
                  p);

    return bm;
}

// Basic test of the SkSpecialImage public API (e.g., peekTexture, peekPixels & draw)
static void test_image(const sk_sp<SkSpecialImage>& img, skiatest::Reporter* reporter,
                       GrContext* context, bool isGPUBacked) {
    const SkIRect subset = img->subset();
    REPORTER_ASSERT(reporter, kPad == subset.left());
    REPORTER_ASSERT(reporter, kPad == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    // Test that isTextureBacked reports the correct backing type
    REPORTER_ASSERT(reporter, isGPUBacked == img->isTextureBacked());

    //--------------
    // Test asTextureProxyRef - as long as there is a context this should succeed
    if (context) {
        sk_sp<GrTextureProxy> proxy(img->asTextureProxyRef(context));
        REPORTER_ASSERT(reporter, proxy);
    }

    //--------------
    // Test getROPixels - this should always succeed regardless of backing store
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, img->getROPixels(&bitmap));
    REPORTER_ASSERT(reporter, kSmallerSize == bitmap.width());
    REPORTER_ASSERT(reporter, kSmallerSize == bitmap.height());

    //--------------
    // Test that draw restricts itself to the subset
    SkImageFilter_Base::OutputProperties outProps(kN32_SkColorType, img->getColorSpace());
    sk_sp<SkSpecialSurface> surf(img->makeSurface(outProps, SkISize::Make(kFullSize, kFullSize),
                                                  kPremul_SkAlphaType));

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorBLUE);
    img->draw(canvas, SkIntToScalar(kPad), SkIntToScalar(kPad), nullptr);

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
        SkImageFilter_Base::OutputProperties outProps(kN32_SkColorType, img->getColorSpace());
        sk_sp<SkSurface> tightSurf(img->makeTightSurface(outProps, subset.size()));

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

    sk_sp<SkImage> fullImage(SkImage::MakeFromBitmap(bm));

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

static void test_texture_backed(skiatest::Reporter* reporter,
                                const sk_sp<SkSpecialImage>& orig,
                                const sk_sp<SkSpecialImage>& gpuBacked) {
    REPORTER_ASSERT(reporter, gpuBacked);
    REPORTER_ASSERT(reporter, gpuBacked->isTextureBacked());
    REPORTER_ASSERT(reporter, gpuBacked->uniqueID() == orig->uniqueID());
    REPORTER_ASSERT(reporter, gpuBacked->subset().width() == orig->subset().width() &&
                              gpuBacked->subset().height() == orig->subset().height());
    REPORTER_ASSERT(reporter, gpuBacked->getColorSpace() == orig->getColorSpace());
}

// Test out the SkSpecialImage::makeTextureImage entry point
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_MakeTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    SkBitmap bm = create_bm();

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        // raster
        sk_sp<SkSpecialImage> rasterImage(SkSpecialImage::MakeFromRaster(
                                                                        SkIRect::MakeWH(kFullSize,
                                                                                        kFullSize),
                                                                        bm));

        {
            sk_sp<SkSpecialImage> fromRaster(rasterImage->makeTextureImage(context));
            test_texture_backed(reporter, rasterImage, fromRaster);
        }

        {
            sk_sp<SkSpecialImage> subRasterImage(rasterImage->makeSubset(subset));

            sk_sp<SkSpecialImage> fromSubRaster(subRasterImage->makeTextureImage(context));
            test_texture_backed(reporter, subRasterImage, fromSubRaster);
        }
    }

    {
        // gpu
        sk_sp<SkImage> rasterImage = SkImage::MakeFromBitmap(bm);
        sk_sp<GrTextureProxy> proxy = proxyProvider->createTextureProxy(
                rasterImage, 1, SkBudgeted::kNo, SkBackingFit::kExact);
        if (!proxy) {
            return;
        }

        sk_sp<SkSpecialImage> gpuImage(SkSpecialImage::MakeDeferredFromGpu(
                                                            context,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            kNeedNewImageUniqueID_SpecialImage,
                                                            std::move(proxy), nullptr));

        {
            sk_sp<SkSpecialImage> fromGPU(gpuImage->makeTextureImage(context));
            test_texture_backed(reporter, gpuImage, fromGPU);
        }

        {
            sk_sp<SkSpecialImage> subGPUImage(gpuImage->makeSubset(subset));

            sk_sp<SkSpecialImage> fromSubGPU(subGPUImage->makeTextureImage(context));
            test_texture_backed(reporter, subGPUImage, fromSubGPU);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    SkBitmap bm = create_bm();
    sk_sp<SkImage> rasterImage = SkImage::MakeFromBitmap(bm);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createTextureProxy(rasterImage, 1, SkBudgeted::kNo,
                                                                    SkBackingFit::kExact);
    if (!proxy) {
        return;
    }

    sk_sp<SkSpecialImage> fullSImg(SkSpecialImage::MakeDeferredFromGpu(
                                                            context,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            kNeedNewImageUniqueID_SpecialImage,
                                                            proxy, nullptr));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeDeferredFromGpu(
                                                               context, subset,
                                                               kNeedNewImageUniqueID_SpecialImage,
                                                               std::move(proxy), nullptr));
        test_image(subSImg1, reporter, context, true);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, context, true);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_ReadbackAndCachingSubsets_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    SkImageInfo ii = SkImageInfo::Make(50, 50, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    // Fill out our surface:
    // Green | Blue
    //  Red  | Green
    {
        surface->getCanvas()->clear(SK_ColorGREEN);
        SkPaint p;
        p.setColor(SK_ColorRED);
        surface->getCanvas()->drawRect(SkRect::MakeXYWH(0, 25, 25, 25), p);
        p.setColor(SK_ColorBLUE);
        surface->getCanvas()->drawRect(SkRect::MakeXYWH(25, 0, 25, 25), p);
    }

    auto image = surface->makeImageSnapshot();
    auto redImg  = SkSpecialImage::MakeFromImage(context, SkIRect::MakeXYWH(10, 30, 10, 10), image);
    auto blueImg = SkSpecialImage::MakeFromImage(context, SkIRect::MakeXYWH(30, 10, 10, 10), image);

    // This isn't necessary, but if it ever becomes false, then the cache collision bug that we're
    // checking below is irrelevant.
    REPORTER_ASSERT(reporter, redImg->uniqueID() == blueImg->uniqueID());

    SkBitmap redBM, blueBM;
    SkAssertResult(redImg->getROPixels(&redBM));
    SkAssertResult(blueImg->getROPixels(&blueBM));

    // Each image should read from the correct sub-rect. Past bugs (skbug.com/8448) have included:
    // - Always reading back from (0, 0), producing green
    // - Incorrectly hitting the cache on the 2nd read-back, causing blueBM to be red
    REPORTER_ASSERT(reporter, redBM.getColor(0, 0) == SK_ColorRED,
                    "0x%08x != 0x%08x", redBM.getColor(0, 0), SK_ColorRED);
    REPORTER_ASSERT(reporter, blueBM.getColor(0, 0) == SK_ColorBLUE,
                    "0x%08x != 0x%08x", blueBM.getColor(0, 0), SK_ColorBLUE);
}
