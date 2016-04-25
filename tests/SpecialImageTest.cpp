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
                       GrContext* context, bool peekTextureSucceeds,
                       int offset, int size) {
    const SkIRect subset = img->subset();
    REPORTER_ASSERT(reporter, offset == subset.left());
    REPORTER_ASSERT(reporter, offset == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    // Test that peekTexture reports the correct backing type
    REPORTER_ASSERT(reporter, peekTextureSucceeds == img->isTextureBacked());

#if SK_SUPPORT_GPU
    //--------------
    // Test getTextureAsRef - as long as there is a context this should succeed
    if (context) {
        sk_sp<GrTexture> texture(img->asTextureRef(context));
        REPORTER_ASSERT(reporter, texture);
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
    SkImageInfo info = SkImageInfo::MakeN32(kFullSize, kFullSize, kOpaque_SkAlphaType);

    sk_sp<SkSpecialSurface> surf(img->makeSurface(info));

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorBLUE);
    img->draw(canvas, SkIntToScalar(kPad), SkIntToScalar(kPad), nullptr);

    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);

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
    // Test that makeTightSubset & makeTightSurface return appropriately sized objects
    // of the correct backing type
    SkIRect newSubset = SkIRect::MakeWH(subset.width(), subset.height());
    {
        sk_sp<SkImage> tightImg(img->makeTightSubset(newSubset));

        REPORTER_ASSERT(reporter, tightImg->width() == subset.width());
        REPORTER_ASSERT(reporter, tightImg->height() == subset.height());
        REPORTER_ASSERT(reporter, peekTextureSucceeds == !!tightImg->getTexture());
        SkPixmap tmpPixmap;
        REPORTER_ASSERT(reporter, peekTextureSucceeds != !!tightImg->peekPixels(&tmpPixmap));
    }
    {
        SkImageInfo info = SkImageInfo::MakeN32(subset.width(), subset.height(),
                                                kPremul_SkAlphaType);
        sk_sp<SkSurface> tightSurf(img->makeTightSurface(info));

        REPORTER_ASSERT(reporter, tightSurf->width() == subset.width());
        REPORTER_ASSERT(reporter, tightSurf->height() == subset.height());
        REPORTER_ASSERT(reporter, peekTextureSucceeds ==
                     !!tightSurf->getTextureHandle(SkSurface::kDiscardWrite_BackendHandleAccess));
        SkPixmap tmpPixmap;
        REPORTER_ASSERT(reporter, peekTextureSucceeds != !!tightSurf->peekPixels(&tmpPixmap));
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

DEF_TEST(SpecialImage_Image, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkImage> fullImage(SkImage::MakeFromBitmap(bm));

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromImage(
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            fullImage));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromImage(subset, fullImage));
        test_image(subSImg1, reporter, nullptr, false, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, nullptr, false, 0, kSmallerSize);
    }
}

DEF_TEST(SpecialImage_Pixmap, reporter) {
    SkAutoPixmapStorage pixmap;

    const SkImageInfo info = SkImageInfo::MakeN32(kFullSize, kFullSize, kOpaque_SkAlphaType);
    pixmap.alloc(info);
    pixmap.erase(SK_ColorGREEN);

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    pixmap.erase(SK_ColorRED, subset);

    {
        sk_sp<SkSpecialImage> img(SkSpecialImage::MakeFromPixmap(subset, pixmap,
                                                                 nullptr, nullptr));
        test_image(img, reporter, nullptr, false, kPad, kFullSize);
    }
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
}

// Test out the SkSpecialImage::makeTextureImage entry point
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_MakeTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.fGrContext;
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
        GrSurfaceDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = kFullSize;
        desc.fHeight = kFullSize;

        sk_sp<GrTexture> texture(context->textureProvider()->createTexture(desc,
                                                                           SkBudgeted::kNo,
                                                                           bm.getPixels(),
                                                                           0));
        if (!texture) {
            return;
        }

        sk_sp<SkSpecialImage> gpuImage(SkSpecialImage::MakeFromGpu(
                                                                SkIRect::MakeWH(kFullSize,
                                                                                kFullSize),
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                std::move(texture)));

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
    GrContext* context = ctxInfo.fGrContext;
    SkBitmap bm = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    sk_sp<GrTexture> texture(context->textureProvider()->createTexture(desc,
                                                                       SkBudgeted::kNo,
                                                                       bm.getPixels(), 0));
    if (!texture) {
        return;
    }

    sk_sp<SkSpecialImage> fullSImg(SkSpecialImage::MakeFromGpu(
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            kNeedNewImageUniqueID_SpecialImage,
                                                            texture));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromGpu(
                                                               subset,
                                                               kNeedNewImageUniqueID_SpecialImage,
                                                               texture));
        test_image(subSImg1, reporter, context, true, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, context, true, kPad, kFullSize);
    }
}

#endif
