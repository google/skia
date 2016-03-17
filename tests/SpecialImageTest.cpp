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
#include "Test.h"
#include "TestingSpecialImageAccess.h"

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
static void test_image(SkSpecialImage* img, skiatest::Reporter* reporter,
                       bool peekPixelsSucceeds, bool peekTextureSucceeds,
                       int offset, int size) {
    const SkIRect subset = TestingSpecialImageAccess::Subset(img);
    REPORTER_ASSERT(reporter, offset == subset.left());
    REPORTER_ASSERT(reporter, offset == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    REPORTER_ASSERT(reporter, peekTextureSucceeds == !!TestingSpecialImageAccess::PeekTexture(img));

    //--------------
    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, peekPixelsSucceeds ==
                              !!TestingSpecialImageAccess::PeekPixels(img, &pixmap));
    if (peekPixelsSucceeds) {
        REPORTER_ASSERT(reporter, size == pixmap.width());
        REPORTER_ASSERT(reporter, size == pixmap.height());
    }

    //--------------
    SkImageInfo info = SkImageInfo::MakeN32(kFullSize, kFullSize, kOpaque_SkAlphaType);

    SkAutoTUnref<SkSpecialSurface> surf(img->newSurface(info));

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
}

DEF_TEST(SpecialImage_Raster, reporter) {
    SkBitmap bm = create_bm();

    SkAutoTUnref<SkSpecialImage> fullSImage(SkSpecialImage::NewFromRaster(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            bm));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        SkAutoTUnref<SkSpecialImage> subSImg1(SkSpecialImage::NewFromRaster(nullptr, subset, bm));
        test_image(subSImg1, reporter, true, false, kPad, kFullSize);
    }

    {
        SkAutoTUnref<SkSpecialImage> subSImg2(fullSImage->extractSubset(subset));
        test_image(subSImg2, reporter, true, false, 0, kSmallerSize);
    }
}

DEF_TEST(SpecialImage_Image, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkImage> fullImage(SkImage::MakeFromBitmap(bm));

    SkAutoTUnref<SkSpecialImage> fullSImage(SkSpecialImage::NewFromImage(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            fullImage.get()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        SkAutoTUnref<SkSpecialImage> subSImg1(SkSpecialImage::NewFromImage(nullptr,
                                                                           subset,
                                                                           fullImage.get()));
        test_image(subSImg1, reporter, true, false, kPad, kFullSize);
    }

    {
        SkAutoTUnref<SkSpecialImage> subSImg2(fullSImage->extractSubset(subset));
        test_image(subSImg2, reporter, true, false, 0, kSmallerSize);
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
        // The SkAutoPixmapStorage keeps hold of the memory
        SkAutoTUnref<SkSpecialImage> img(SkSpecialImage::NewFromPixmap(nullptr, subset, pixmap,
                                                                       nullptr, nullptr));
        test_image(img, reporter, true, false, kPad, kFullSize);
    }

    {
        // The image takes ownership of the memory
        SkAutoTUnref<SkSpecialImage> img(SkSpecialImage::NewFromPixmap(
                                               nullptr, subset, pixmap,
                                               [] (void* addr, void*) -> void { sk_free(addr); },
                                               nullptr));
        pixmap.release();
        test_image(img, reporter, true, false, kPad, kFullSize);
    }
}


#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_Gpu, reporter, context) {
    SkBitmap bm = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    SkAutoTUnref<GrTexture> texture(context->textureProvider()->createTexture(desc, SkBudgeted::kNo,
                                                                              bm.getPixels(), 0));
    if (!texture) {
        return;
    }

    SkAutoTUnref<SkSpecialImage> fullSImg(SkSpecialImage::NewFromGpu(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            kNeedNewImageUniqueID_SpecialImage,
                                                            texture));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        SkAutoTUnref<SkSpecialImage> subSImg1(SkSpecialImage::NewFromGpu(
                                                               nullptr, subset, 
                                                               kNeedNewImageUniqueID_SpecialImage,
                                                               texture));
        test_image(subSImg1, reporter, false, true, kPad, kFullSize);
    }

    {
        SkAutoTUnref<SkSpecialImage> subSImg2(fullSImg->extractSubset(subset));
        test_image(subSImg2, reporter, false, true, kPad, kFullSize);
    }
}

#endif
