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
