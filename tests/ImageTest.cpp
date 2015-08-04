/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkImage_Base.h"
#include "SkRRect.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrTest.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#else
class GrContextFactory;
class GrContext;
#endif

static void assert_equal(skiatest::Reporter* reporter, SkImage* a, const SkIRect* subsetA,
                         SkImage* b) {
    const int widthA = subsetA ? subsetA->width() : a->width();
    const int heightA = subsetA ? subsetA->height() : a->height();

    REPORTER_ASSERT(reporter, widthA == b->width());
    REPORTER_ASSERT(reporter, heightA == b->height());
#if 0
    // see skbug.com/3965
    bool AO = a->isOpaque();
    bool BO = b->isOpaque();
    REPORTER_ASSERT(reporter, AO == BO);
#endif

    SkImageInfo info = SkImageInfo::MakeN32(widthA, heightA,
                                        a->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);

    const int srcX = subsetA ? subsetA->x() : 0;
    const int srcY = subsetA ? subsetA->y() : 0;

    REPORTER_ASSERT(reporter, a->readPixels(pmapA, srcX, srcY));
    REPORTER_ASSERT(reporter, b->readPixels(pmapB, 0, 0));

    const size_t widthBytes = widthA * info.bytesPerPixel();
    for (int y = 0; y < heightA; ++y) {
        REPORTER_ASSERT(reporter, !memcmp(pmapA.addr32(0, y), pmapB.addr32(0, y), widthBytes));
    }
}

static SkImage* make_image(GrContext* ctx, int w, int h, const SkIRect& ir) {
    const SkImageInfo info = SkImageInfo::MakeN32(w, h, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(ctx ?
                                    SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info) :
                                    SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::Make(ir), paint);
    return surface->newImageSnapshot();
}

static void test_encode(skiatest::Reporter* reporter, GrContext* ctx) {
    const SkIRect ir = SkIRect::MakeXYWH(5, 5, 10, 10);
    SkAutoTUnref<SkImage> orig(make_image(ctx, 20, 20, ir));
    SkAutoTUnref<SkData> origEncoded(orig->encode());
    REPORTER_ASSERT(reporter, origEncoded);
    REPORTER_ASSERT(reporter, origEncoded->size() > 0);

    SkAutoTUnref<SkImage> decoded(SkImage::NewFromEncoded(origEncoded));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, orig, NULL, decoded);

    // Now see if we can instantiate an image from a subset of the surface/origEncoded
    
    decoded.reset(SkImage::NewFromEncoded(origEncoded, &ir));
    REPORTER_ASSERT(reporter, decoded);
    assert_equal(reporter, orig, &ir, decoded);
}

DEF_TEST(Image_Encode_Cpu, reporter) {
    test_encode(reporter, NULL);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST(Image_Encode_Gpu, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }
    test_encode(reporter, ctx);
}
#endif

DEF_TEST(Image_NewRasterCopy, reporter) {
    const SkPMColor red =   SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkPMColor green = SkPackARGB32(0xFF, 0, 0xFF, 0);
    const SkPMColor blue =  SkPackARGB32(0xFF, 0, 0, 0xFF);
    SkPMColor colors[] = { red, green, blue, 0 };
    SkAutoTUnref<SkColorTable> ctable(SkNEW_ARGS(SkColorTable, (colors, SK_ARRAY_COUNT(colors))));
    // The colortable made a copy, so we can trash the original colors
    memset(colors, 0xFF, sizeof(colors));

    const SkImageInfo srcInfo = SkImageInfo::Make(2, 2, kIndex_8_SkColorType, kPremul_SkAlphaType);
    const size_t srcRowBytes = 2 * sizeof(uint8_t);
    uint8_t indices[] = { 0, 1, 2, 3 };
    SkAutoTUnref<SkImage> image(SkImage::NewRasterCopy(srcInfo, indices, srcRowBytes, ctable));
    // The image made a copy, so we can trash the original indices
    memset(indices, 0xFF, sizeof(indices));

    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);
    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    image->readPixels(dstInfo, pixels, dstRowBytes, 0, 0);
    REPORTER_ASSERT(reporter, red == pixels[0]);
    REPORTER_ASSERT(reporter, green == pixels[1]);
    REPORTER_ASSERT(reporter, blue == pixels[2]);
    REPORTER_ASSERT(reporter, 0 == pixels[3]);
}

// Test that a draw that only partially covers the drawing surface isn't
// interpreted as covering the entire drawing surface (i.e., exercise one of the
// conditions of SkCanvas::wouldOverwriteEntireSurface()).
DEF_TEST(Image_RetainSnapshot, reporter) {
    const SkPMColor red   = SkPackARGB32(0xFF, 0xFF, 0, 0);
    const SkPMColor green = SkPackARGB32(0xFF, 0, 0xFF, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    surface->getCanvas()->clear(0xFF00FF00);

    SkPMColor pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));   // init with values we don't expect
    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(2, 2);
    const size_t dstRowBytes = 2 * sizeof(SkPMColor);

    SkAutoTUnref<SkImage> image1(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image1->readPixels(dstInfo, pixels, dstRowBytes, 0, 0));
    for (size_t i = 0; i < SK_ARRAY_COUNT(pixels); ++i) {
        REPORTER_ASSERT(reporter, pixels[i] == green);
    }

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColor(SK_ColorRED);

    surface->getCanvas()->drawRect(SkRect::MakeXYWH(1, 1, 1, 1), paint);

    SkAutoTUnref<SkImage> image2(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image2->readPixels(dstInfo, pixels, dstRowBytes, 0, 0));
    REPORTER_ASSERT(reporter, pixels[0] == green);
    REPORTER_ASSERT(reporter, pixels[1] == green);
    REPORTER_ASSERT(reporter, pixels[2] == green);
    REPORTER_ASSERT(reporter, pixels[3] == red);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkImageGenerator.h"
#include "SkData.h"

const uint8_t tiny_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x64,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x70, 0xe2, 0x95, 0x54, 0x00, 0x00, 0x00,
    0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00,
    0x01, 0x6b, 0x49, 0x44, 0x41, 0x54, 0x78, 0x01, 0xed, 0xd3, 0x41, 0x11,
    0x00, 0x20, 0x0c, 0xc4, 0x40, 0xc0, 0xbf, 0xe7, 0xc2, 0xa0, 0x22, 0x8f,
    0xad, 0x82, 0x4c, 0xd2, 0xdb, 0xf3, 0x6e, 0xb9, 0x8c, 0x81, 0x93, 0x21,
    0x01, 0xf2, 0x0d, 0x08, 0x12, 0x7b, 0x04, 0x41, 0x04, 0x89, 0x19, 0x88,
    0xe1, 0x58, 0x88, 0x20, 0x31, 0x03, 0x31, 0x1c, 0x0b, 0x11, 0x24, 0x66,
    0x20, 0x86, 0x63, 0x21, 0x82, 0xc4, 0x0c, 0xc4, 0x70, 0x2c, 0x44, 0x90,
    0x98, 0x81, 0x18, 0x8e, 0x85, 0x08, 0x12, 0x33, 0x10, 0xc3, 0xb1, 0x10,
    0x41, 0x62, 0x06, 0x62, 0x38, 0x16, 0x22, 0x48, 0xcc, 0x40, 0x0c, 0xc7,
    0x42, 0x04, 0x89, 0x19, 0x88, 0xe1, 0x58, 0x88, 0x20, 0x31, 0x03, 0x31,
    0x1c, 0x0b, 0x11, 0x24, 0x66, 0x20, 0x86, 0x63, 0x21, 0x82, 0xc4, 0x0c,
    0xc4, 0x70, 0x2c, 0x44, 0x90, 0x98, 0x81, 0x18, 0x8e, 0x85, 0x08, 0x12,
    0x33, 0x10, 0xc3, 0xb1, 0x10, 0x41, 0x62, 0x06, 0x62, 0x38, 0x16, 0x22,
    0x48, 0xcc, 0x40, 0x0c, 0xc7, 0x42, 0x04, 0x89, 0x19, 0x88, 0xe1, 0x58,
    0x88, 0x20, 0x31, 0x03, 0x31, 0x1c, 0x0b, 0x11, 0x24, 0x66, 0x20, 0x86,
    0x63, 0x21, 0x82, 0xc4, 0x0c, 0xc4, 0x70, 0x2c, 0x44, 0x90, 0x98, 0x81,
    0x18, 0x8e, 0x85, 0x08, 0x12, 0x33, 0x10, 0xc3, 0xb1, 0x10, 0x41, 0x62,
    0x06, 0x62, 0x38, 0x16, 0x22, 0x48, 0xcc, 0x40, 0x0c, 0xc7, 0x42, 0x04,
    0x89, 0x19, 0x88, 0xe1, 0x58, 0x88, 0x20, 0x31, 0x03, 0x31, 0x1c, 0x0b,
    0x11, 0x24, 0x66, 0x20, 0x86, 0x63, 0x21, 0x82, 0xc4, 0x0c, 0xc4, 0x70,
    0x2c, 0x44, 0x90, 0x98, 0x81, 0x18, 0x8e, 0x85, 0x08, 0x12, 0x33, 0x10,
    0xc3, 0xb1, 0x10, 0x41, 0x62, 0x06, 0x62, 0x38, 0x16, 0x22, 0x48, 0xcc,
    0x40, 0x0c, 0xc7, 0x42, 0x04, 0x89, 0x19, 0x88, 0xe1, 0x58, 0x88, 0x20,
    0x31, 0x03, 0x31, 0x1c, 0x0b, 0x11, 0x24, 0x66, 0x20, 0x86, 0x63, 0x21,
    0x82, 0xc4, 0x0c, 0xc4, 0x70, 0x2c, 0x44, 0x90, 0x98, 0x81, 0x18, 0x8e,
    0x85, 0x08, 0x12, 0x33, 0x10, 0xc3, 0xb1, 0x10, 0x41, 0x62, 0x06, 0x62,
    0x38, 0x16, 0x22, 0x48, 0xcc, 0x40, 0x0c, 0xc7, 0x42, 0x04, 0x89, 0x19,
    0x88, 0xe1, 0x58, 0x88, 0x20, 0x31, 0x03, 0x31, 0x1c, 0x0b, 0x11, 0x24,
    0x66, 0x20, 0x86, 0x63, 0x21, 0x82, 0xc4, 0x0c, 0xc4, 0x70, 0x2c, 0x44,
    0x90, 0x98, 0x81, 0x18, 0x8e, 0x85, 0x08, 0x12, 0x33, 0x10, 0xc3, 0xb1,
    0x10, 0x41, 0x62, 0x06, 0x62, 0x38, 0x16, 0x22, 0x48, 0xcc, 0x40, 0x0c,
    0xc7, 0x42, 0x62, 0x41, 0x2e, 0x08, 0x60, 0x04, 0xc4, 0x4c, 0x5d, 0x6e,
    0xf2, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
    0x82
};

static void make_bitmap_lazy(SkBitmap* bm) {
    SkAutoTUnref<SkData> data(SkData::NewWithoutCopy(tiny_png, sizeof(tiny_png)));
    SkInstallDiscardablePixelRef(data, bm);
}

static void make_bitmap_mutable(SkBitmap* bm) {
    bm->allocN32Pixels(10, 10);
}

static void make_bitmap_immutable(SkBitmap* bm) {
    bm->allocN32Pixels(10, 10);
    bm->setImmutable();
}

DEF_TEST(image_newfrombitmap, reporter) {
    const struct {
        void (*fMakeProc)(SkBitmap*);
        bool fExpectPeekSuccess;
        bool fExpectSharedID;
    } rec[] = {
        { make_bitmap_lazy,         false,  true  },
        { make_bitmap_mutable,      true,   false },
        { make_bitmap_immutable,    true,   true  },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
        SkBitmap bm;
        rec[i].fMakeProc(&bm);

        SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bm));
        SkPixmap pmap;

        const bool sharedID = (image->uniqueID() == bm.getGenerationID());
        REPORTER_ASSERT(reporter, sharedID == rec[i].fExpectSharedID);

        const bool peekSuccess = image->peekPixels(&pmap);
        REPORTER_ASSERT(reporter, peekSuccess == rec[i].fExpectPeekSuccess);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU

static SkImage* make_gpu_image(GrContext* ctx, const SkImageInfo& info, SkColor color) {
    const SkSurface::Budgeted budgeted = SkSurface::kNo_Budgeted;
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx, budgeted, info, 0));
    surface->getCanvas()->drawColor(color);
    return surface->newImageSnapshot();
}

#include "SkBitmapCache.h"

/*
 *  This tests the caching (and preemptive purge) of the raster equivalent of a gpu-image.
 *  We cache it for performance when drawing into a raster surface.
 *
 *  A cleaner test would know if each drawImage call triggered a read-back from the gpu,
 *  but we don't have that facility (at the moment) so we use a little internal knowledge
 *  of *how* the raster version is cached, and look for that.
 */
DEF_GPUTEST(SkImage_Gpu2Cpu, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    if (!ctx) {
        REPORTER_ASSERT(reporter, false);
        return;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    SkAutoTUnref<SkImage> image(make_gpu_image(ctx, info, SK_ColorRED));
    const uint32_t uniqueID = image->uniqueID();

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));

    // now we can test drawing a gpu-backed image into a cpu-backed surface

    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(uniqueID, &cachedBitmap));
    }

    surface->getCanvas()->drawImage(image, 0, 0);
    {
        SkBitmap cachedBitmap;
        if (SkBitmapCache::Find(uniqueID, &cachedBitmap)) {
            REPORTER_ASSERT(reporter, cachedBitmap.getGenerationID() == uniqueID);
            REPORTER_ASSERT(reporter, cachedBitmap.isImmutable());
            REPORTER_ASSERT(reporter, cachedBitmap.getPixels());
        } else {
            // unexpected, but not really a bug, since the cache is global and this test may be
            // run w/ other threads competing for its budget.
            SkDebugf("SkImage_Gpu2Cpu : cachedBitmap was already purged\n");
        }
    }

    image.reset(nullptr);
    {
        SkBitmap cachedBitmap;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(uniqueID, &cachedBitmap));
    }
}
#endif
