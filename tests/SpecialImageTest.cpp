/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

class TestingSpecialImageAccess {
public:
    static const SkIRect& Subset(const SkSpecialImage* img) {
        return img->subset();
    }

    static bool PeekPixels(const SkSpecialImage* img, SkPixmap* pixmap) {
        return img->peekPixels(pixmap);
    }

    static GrTexture* PeekTexture(const SkSpecialImage* img) {
        return img->peekTexture();
    }
};

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
                       bool peekPixelsSucceeds, bool peekTextureSucceeds) {
    const SkIRect subset = TestingSpecialImageAccess::Subset(img);
    REPORTER_ASSERT(reporter, kPad == subset.left());
    REPORTER_ASSERT(reporter, kPad == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    REPORTER_ASSERT(reporter, peekTextureSucceeds == !!TestingSpecialImageAccess::PeekTexture(img));

    //--------------
    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, peekPixelsSucceeds ==
                              !!TestingSpecialImageAccess::PeekPixels(img, &pixmap));
    if (peekPixelsSucceeds) {
        REPORTER_ASSERT(reporter, kFullSize == pixmap.width());
        REPORTER_ASSERT(reporter, kFullSize == pixmap.height());
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

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> img(SkSpecialImage::NewFromRaster(nullptr, subset, bm));
    test_image(img, reporter, true, false);
}

DEF_TEST(SpecialImage_Image, reporter) {
    SkBitmap bm = create_bm();

    SkAutoTUnref<SkImage> fullImage(SkImage::NewFromBitmap(bm));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> img(SkSpecialImage::NewFromImage(subset, fullImage));
    test_image(img, reporter, true, false);
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

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> img(SkSpecialImage::NewFromGpu(nullptr, subset, 
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                texture));
    test_image(img, reporter, false, true);
}

#endif
