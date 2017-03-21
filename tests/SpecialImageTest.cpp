/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPixmap.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkSurface.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#endif


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
                       GrContext* context, bool isGPUBacked,
                       int offset, int size) {
    const SkIRect subset = img->subset();
    REPORTER_ASSERT(reporter, offset == subset.left());
    REPORTER_ASSERT(reporter, offset == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    // Test that isTextureBacked reports the correct backing type
    REPORTER_ASSERT(reporter, isGPUBacked == img->isTextureBacked());

#if SK_SUPPORT_GPU
    //--------------
    // Test asTextureProxyRef - as long as there is a context this should succeed
    if (context) {
        sk_sp<GrTextureProxy> proxy(img->asTextureProxyRef(context));
        REPORTER_ASSERT(reporter, proxy);
    }
#endif

    //--------------
    // Test getROPixels - this should always succeed regardless of backing store
    SkBitmap bitmap;
    REPORTER_ASSERT(reporter, img->getROPixels(&bitmap));
    if (context) {
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.width());
        REPORTER_ASSERT(reporter, kSmallerSize == bitmap.height());
    } else {
        REPORTER_ASSERT(reporter, size == bitmap.width());
        REPORTER_ASSERT(reporter, size == bitmap.height());
    }

    //--------------
    // Test that draw restricts itself to the subset
    SkImageFilter::OutputProperties outProps(img->getColorSpace());
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
        SkImageFilter::OutputProperties outProps(img->getColorSpace());
        sk_sp<SkSurface> tightSurf(img->makeTightSurface(outProps, subset.size()));

        REPORTER_ASSERT(reporter, tightSurf->width() == subset.width());
        REPORTER_ASSERT(reporter, tightSurf->height() == subset.height());
        REPORTER_ASSERT(reporter, isGPUBacked ==
                     !!tightSurf->getTextureHandle(SkSurface::kDiscardWrite_BackendHandleAccess));
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
        test_image(subSImg1, reporter, nullptr, false, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, nullptr, false, 0, kSmallerSize);
    }
}

static void test_specialimage_image(skiatest::Reporter* reporter, SkColorSpace* dstColorSpace) {
    SkBitmap bm = create_bm();

    sk_sp<SkImage> fullImage(SkImage::MakeFromBitmap(bm));

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromImage(
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            fullImage, dstColorSpace));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromImage(subset, fullImage,
                                                                     dstColorSpace));
        test_image(subSImg1, reporter, nullptr, false, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, nullptr, false, 0, kSmallerSize);
    }
}

DEF_TEST(SpecialImage_Image_Legacy, reporter) {
    SkColorSpace* legacyColorSpace = nullptr;
    test_specialimage_image(reporter, legacyColorSpace);
}

DEF_TEST(SpecialImage_Image_ColorSpaceAware, reporter) {
    sk_sp<SkColorSpace> srgbColorSpace = SkColorSpace::MakeSRGB();
    test_specialimage_image(reporter, srgbColorSpace.get());
}

#if SK_SUPPORT_GPU

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
        const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bm.info(), *context->caps());

        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                 desc, SkBudgeted::kNo,
                                                                 bm.getPixels(), bm.rowBytes()));
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
    SkBitmap bm = create_bm();

    const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bm.info(), *context->caps());

    sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                             desc, SkBudgeted::kNo,
                                                             bm.getPixels(), bm.rowBytes()));
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
        test_image(subSImg1, reporter, context, true, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, context, true, kPad, kFullSize);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_DeferredGpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    SkBitmap bm = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                             desc, SkBudgeted::kNo,
                                                             bm.getPixels(), 0));
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
        test_image(subSImg1, reporter, context, true, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, context, true, kPad, kFullSize);
    }
}

#endif
