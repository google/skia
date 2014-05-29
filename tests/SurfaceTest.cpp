/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkRRect.h"
#include "SkSurface.h"
#include "SkUtils.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#else
class GrContextFactory;
class GrContext;
#endif

enum SurfaceType {
    kRaster_SurfaceType,
    kRasterDirect_SurfaceType,
    kGpu_SurfaceType,
    kGpuScratch_SurfaceType,
};

static const int gSurfaceSize = 10;
static SkPMColor gSurfaceStorage[gSurfaceSize * gSurfaceSize];

static SkSurface* createSurface(SurfaceType surfaceType, GrContext* context,
                                SkImageInfo* requestedInfo = NULL) {
    static const SkImageInfo info = SkImageInfo::MakeN32Premul(gSurfaceSize,
                                                               gSurfaceSize);

    if (requestedInfo) {
        *requestedInfo = info;
    }

    switch (surfaceType) {
        case kRaster_SurfaceType:
            return SkSurface::NewRaster(info);
        case kRasterDirect_SurfaceType:
            return SkSurface::NewRasterDirect(info, gSurfaceStorage,
                                              info.minRowBytes());
        case kGpu_SurfaceType:
#if SK_SUPPORT_GPU
            return context ? SkSurface::NewRenderTarget(context, info) : NULL;
#endif
            break;
        case kGpuScratch_SurfaceType:
#if SK_SUPPORT_GPU
            return context ? SkSurface::NewScratchRenderTarget(context, info) : NULL;
#endif
            break;
    }
    return NULL;
}

enum ImageType {
    kRasterCopy_ImageType,
    kRasterData_ImageType,
    kGpu_ImageType,
    kCodec_ImageType,
};

static void test_image(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    void* addr = sk_malloc_throw(size);
    SkData* data = SkData::NewFromMalloc(addr, size);

    REPORTER_ASSERT(reporter, 1 == data->getRefCnt());
    SkImage* image = SkImage::NewRasterData(info, data, rowBytes);
    REPORTER_ASSERT(reporter, 2 == data->getRefCnt());
    image->unref();
    REPORTER_ASSERT(reporter, 1 == data->getRefCnt());
    data->unref();
}

static SkImage* createImage(ImageType imageType, GrContext* context,
                            SkColor color) {
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t rowBytes = info.minRowBytes();
    const size_t size = rowBytes * info.fHeight;

    void* addr = sk_malloc_throw(size);
    sk_memset32((SkPMColor*)addr, pmcolor, SkToInt(size >> 2));
    SkAutoTUnref<SkData> data(SkData::NewFromMalloc(addr, size));

    switch (imageType) {
        case kRasterCopy_ImageType:
            return SkImage::NewRasterCopy(info, addr, rowBytes);
        case kRasterData_ImageType:
            return SkImage::NewRasterData(info, data, rowBytes);
        case kGpu_ImageType:
            return NULL;        // TODO
        case kCodec_ImageType: {
            SkBitmap bitmap;
            bitmap.installPixels(info, addr, rowBytes);
            SkAutoTUnref<SkData> src(
                 SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type,
                                            100));
            return SkImage::NewEncodedData(src);
        }
    }
    SkASSERT(false);
    return NULL;
}

static void test_imagepeek(skiatest::Reporter* reporter) {
    static const struct {
        ImageType   fType;
        bool        fPeekShouldSucceed;
    } gRec[] = {
        { kRasterCopy_ImageType,    true    },
        { kRasterData_ImageType,    true    },
        { kGpu_ImageType,           false   },
        { kCodec_ImageType,         false   },
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkImageInfo info;
        size_t rowBytes;

        SkAutoTUnref<SkImage> image(createImage(gRec[i].fType, NULL, color));
        if (!image.get()) {
            continue;   // gpu may not be enabled
        }
        const void* addr = image->peekPixels(&info, &rowBytes);
        bool success = (NULL != addr);
        REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);
        if (success) {
            REPORTER_ASSERT(reporter, 10 == info.fWidth);
            REPORTER_ASSERT(reporter, 10 == info.fHeight);
            REPORTER_ASSERT(reporter, kN32_SkColorType == info.fColorType);
            REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.fAlphaType ||
                            kOpaque_SkAlphaType == info.fAlphaType);
            REPORTER_ASSERT(reporter, info.minRowBytes() <= rowBytes);
            REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);
        }
    }
}

static void test_canvaspeek(skiatest::Reporter* reporter,
                            GrContextFactory* factory) {
    static const struct {
        SurfaceType fType;
        bool        fPeekShouldSucceed;
    } gRec[] = {
        { kRaster_SurfaceType,          true    },
        { kRasterDirect_SurfaceType,    true    },
#if SK_SUPPORT_GPU
        { kGpu_SurfaceType,             false   },
        { kGpuScratch_SurfaceType,      false   },
#endif
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    GrContext* context = NULL;
#if SK_SUPPORT_GPU
    context = factory->get(GrContextFactory::kNative_GLContextType);
#endif

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkImageInfo info, requestInfo;
        size_t rowBytes;

        SkAutoTUnref<SkSurface> surface(createSurface(gRec[i].fType, context,
                                                      &requestInfo));
        surface->getCanvas()->clear(color);

        const void* addr = surface->getCanvas()->peekPixels(&info, &rowBytes);
        bool success = (NULL != addr);
        REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);

        SkImageInfo info2;
        size_t rb2;
        const void* addr2 = surface->peekPixels(&info2, &rb2);

        if (success) {
            REPORTER_ASSERT(reporter, requestInfo == info);
            REPORTER_ASSERT(reporter, requestInfo.minRowBytes() <= rowBytes);
            REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);

            REPORTER_ASSERT(reporter, addr2 == addr);
            REPORTER_ASSERT(reporter, info2 == info);
            REPORTER_ASSERT(reporter, rb2 == rowBytes);
        } else {
            REPORTER_ASSERT(reporter, NULL == addr2);
        }
    }
}

static void TestSurfaceCopyOnWrite(skiatest::Reporter* reporter, SurfaceType surfaceType,
                                   GrContext* context) {
    // Verify that the right canvas commands trigger a copy on write
    SkSurface* surface = createSurface(surfaceType, context);
    SkAutoTUnref<SkSurface> aur_surface(surface);
    SkCanvas* canvas = surface->getCanvas();

    const SkRect testRect =
        SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                         SkIntToScalar(4), SkIntToScalar(5));
    SkMatrix testMatrix;
    testMatrix.reset();
    testMatrix.setScale(SkIntToScalar(2), SkIntToScalar(3));

    SkPath testPath;
    testPath.addRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                      SkIntToScalar(2), SkIntToScalar(1)));

    const SkIRect testIRect = SkIRect::MakeXYWH(0, 0, 2, 1);

    SkRegion testRegion;
    testRegion.setRect(testIRect);


    const SkColor testColor = 0x01020304;
    const SkPaint testPaint;
    const SkPoint testPoints[3] = {
        {SkIntToScalar(0), SkIntToScalar(0)},
        {SkIntToScalar(2), SkIntToScalar(1)},
        {SkIntToScalar(0), SkIntToScalar(2)}
    };
    const size_t testPointCount = 3;

    SkBitmap testBitmap;
    testBitmap.allocN32Pixels(10, 10);
    testBitmap.eraseColor(0);

    SkRRect testRRect;
    testRRect.setRectXY(testRect, SK_Scalar1, SK_Scalar1);

    SkString testText("Hello World");
    const SkPoint testPoints2[] = {
        { SkIntToScalar(0), SkIntToScalar(1) },
        { SkIntToScalar(1), SkIntToScalar(1) },
        { SkIntToScalar(2), SkIntToScalar(1) },
        { SkIntToScalar(3), SkIntToScalar(1) },
        { SkIntToScalar(4), SkIntToScalar(1) },
        { SkIntToScalar(5), SkIntToScalar(1) },
        { SkIntToScalar(6), SkIntToScalar(1) },
        { SkIntToScalar(7), SkIntToScalar(1) },
        { SkIntToScalar(8), SkIntToScalar(1) },
        { SkIntToScalar(9), SkIntToScalar(1) },
        { SkIntToScalar(10), SkIntToScalar(1) },
    };

#define EXPECT_COPY_ON_WRITE(command)                               \
    {                                                               \
        SkImage* imageBefore = surface->newImageSnapshot();         \
        SkAutoTUnref<SkImage> aur_before(imageBefore);              \
        canvas-> command ;                                          \
        SkImage* imageAfter = surface->newImageSnapshot();          \
        SkAutoTUnref<SkImage> aur_after(imageAfter);                \
        REPORTER_ASSERT(reporter, imageBefore != imageAfter);       \
    }

    EXPECT_COPY_ON_WRITE(clear(testColor))
    EXPECT_COPY_ON_WRITE(drawPaint(testPaint))
    EXPECT_COPY_ON_WRITE(drawPoints(SkCanvas::kPoints_PointMode, testPointCount, testPoints, \
        testPaint))
    EXPECT_COPY_ON_WRITE(drawOval(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRect(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRRect(testRRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawPath(testPath, testPaint))
    EXPECT_COPY_ON_WRITE(drawBitmap(testBitmap, 0, 0))
    EXPECT_COPY_ON_WRITE(drawBitmapRect(testBitmap, NULL, testRect))
    EXPECT_COPY_ON_WRITE(drawBitmapMatrix(testBitmap, testMatrix, NULL))
    EXPECT_COPY_ON_WRITE(drawBitmapNine(testBitmap, testIRect, testRect, NULL))
    EXPECT_COPY_ON_WRITE(drawSprite(testBitmap, 0, 0, NULL))
    EXPECT_COPY_ON_WRITE(drawText(testText.c_str(), testText.size(), 0, 1, testPaint))
    EXPECT_COPY_ON_WRITE(drawPosText(testText.c_str(), testText.size(), testPoints2, \
        testPaint))
    EXPECT_COPY_ON_WRITE(drawTextOnPath(testText.c_str(), testText.size(), testPath, NULL, \
        testPaint))
}

static void TestSurfaceWritableAfterSnapshotRelease(skiatest::Reporter* reporter,
                                                    SurfaceType surfaceType,
                                                    GrContext* context) {
    // This test succeeds by not triggering an assertion.
    // The test verifies that the surface remains writable (usable) after
    // acquiring and releasing a snapshot without triggering a copy on write.
    SkAutoTUnref<SkSurface> surface(createSurface(surfaceType, context));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(1);
    surface->newImageSnapshot()->unref();  // Create and destroy SkImage
    canvas->clear(2);  // Must not assert internally
}

#if SK_SUPPORT_GPU
static void TestSurfaceInCache(skiatest::Reporter* reporter,
                               SurfaceType surfaceType,
                               GrContext* context) {
    context->freeGpuResources();
    int resourceCount;

    context->getResourceCacheUsage(&resourceCount, NULL);
    REPORTER_ASSERT(reporter, 0 == resourceCount);
    SkAutoTUnref<SkSurface> surface(createSurface(surfaceType, context));
    // Note: the stencil buffer is always cached, so kGpu_SurfaceType uses
    // one cached resource, and kGpuScratch_SurfaceType uses two.
    int expectedCachedResources = surfaceType == kGpuScratch_SurfaceType ? 2 : 1;
    context->getResourceCacheUsage(&resourceCount, NULL);
    REPORTER_ASSERT(reporter, expectedCachedResources == resourceCount);

    // Verify that all the cached resources are locked in cache.
    context->freeGpuResources();
    context->getResourceCacheUsage(&resourceCount, NULL);
    REPORTER_ASSERT(reporter, expectedCachedResources == resourceCount);

    // Verify that all the cached resources are unlocked upon surface release
    surface.reset(0);
    context->freeGpuResources();
    context->getResourceCacheUsage(&resourceCount, NULL);
    REPORTER_ASSERT(reporter, 0 == resourceCount);
}

static void Test_crbug263329(skiatest::Reporter* reporter,
                             SurfaceType surfaceType,
                             GrContext* context) {
    // This is a regression test for crbug.com/263329
    // Bug was caused by onCopyOnWrite releasing the old surface texture
    // back to the scratch texture pool even though the texture is used
    // by and active SkImage_Gpu.
    SkAutoTUnref<SkSurface> surface1(createSurface(surfaceType, context));
    SkAutoTUnref<SkSurface> surface2(createSurface(surfaceType, context));
    SkCanvas* canvas1 = surface1->getCanvas();
    SkCanvas* canvas2 = surface2->getCanvas();
    canvas1->clear(1);
    SkAutoTUnref<SkImage> image1(surface1->newImageSnapshot());
    // Trigger copy on write, new backing is a scratch texture
    canvas1->clear(2);
    SkAutoTUnref<SkImage> image2(surface1->newImageSnapshot());
    // Trigger copy on write, old backing should not be returned to scratch
    // pool because it is held by image2
    canvas1->clear(3);

    canvas2->clear(4);
    SkAutoTUnref<SkImage> image3(surface2->newImageSnapshot());
    // Trigger copy on write on surface2. The new backing store should not
    // be recycling a texture that is held by an existing image.
    canvas2->clear(5);
    SkAutoTUnref<SkImage> image4(surface2->newImageSnapshot());
    REPORTER_ASSERT(reporter, image4->getTexture() != image3->getTexture());
    // The following assertion checks crbug.com/263329
    REPORTER_ASSERT(reporter, image4->getTexture() != image2->getTexture());
    REPORTER_ASSERT(reporter, image4->getTexture() != image1->getTexture());
    REPORTER_ASSERT(reporter, image3->getTexture() != image2->getTexture());
    REPORTER_ASSERT(reporter, image3->getTexture() != image1->getTexture());
    REPORTER_ASSERT(reporter, image2->getTexture() != image1->getTexture());
}

static void TestGetTexture(skiatest::Reporter* reporter,
                                 SurfaceType surfaceType,
                                 GrContext* context) {
    SkAutoTUnref<SkSurface> surface(createSurface(surfaceType, context));
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    GrTexture* texture = image->getTexture();
    if (surfaceType == kGpu_SurfaceType || surfaceType == kGpuScratch_SurfaceType) {
        REPORTER_ASSERT(reporter, NULL != texture);
        REPORTER_ASSERT(reporter, 0 != texture->getTextureHandle());
    } else {
        REPORTER_ASSERT(reporter, NULL == texture);
    }
    surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
    REPORTER_ASSERT(reporter, image->getTexture() == texture);
}
#endif

static void TestSurfaceNoCanvas(skiatest::Reporter* reporter,
                                          SurfaceType surfaceType,
                                          GrContext* context,
                                          SkSurface::ContentChangeMode mode) {
    // Verifies the robustness of SkSurface for handling use cases where calls
    // are made before a canvas is created.
    {
        // Test passes by not asserting
        SkSurface* surface = createSurface(surfaceType, context);
        SkAutoTUnref<SkSurface> aur_surface(surface);
        surface->notifyContentWillChange(mode);
        SkDEBUGCODE(surface->validate();)
    }
    {
        SkSurface* surface = createSurface(surfaceType, context);
        SkAutoTUnref<SkSurface> aur_surface(surface);
        SkImage* image1 = surface->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_image1(image1);
        SkDEBUGCODE(image1->validate();)
        SkDEBUGCODE(surface->validate();)
        surface->notifyContentWillChange(mode);
        SkDEBUGCODE(image1->validate();)
        SkDEBUGCODE(surface->validate();)
        SkImage* image2 = surface->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_image2(image2);
        SkDEBUGCODE(image2->validate();)
        SkDEBUGCODE(surface->validate();)
        REPORTER_ASSERT(reporter, image1 != image2);
    }

}

DEF_GPUTEST(Surface, reporter, factory) {
    test_image(reporter);

    TestSurfaceCopyOnWrite(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kDiscard_ContentChangeMode);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kRetain_ContentChangeMode);

    test_imagepeek(reporter);
    test_canvaspeek(reporter, factory);

#if SK_SUPPORT_GPU
    TestGetTexture(reporter, kRaster_SurfaceType, NULL);
    if (NULL != factory) {
        GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);
        if (NULL != context) {
            TestSurfaceInCache(reporter, kGpu_SurfaceType, context);
            TestSurfaceInCache(reporter, kGpuScratch_SurfaceType, context);
            Test_crbug263329(reporter, kGpu_SurfaceType, context);
            Test_crbug263329(reporter, kGpuScratch_SurfaceType, context);
            TestSurfaceCopyOnWrite(reporter, kGpu_SurfaceType, context);
            TestSurfaceCopyOnWrite(reporter, kGpuScratch_SurfaceType, context);
            TestSurfaceWritableAfterSnapshotRelease(reporter, kGpu_SurfaceType, context);
            TestSurfaceWritableAfterSnapshotRelease(reporter, kGpuScratch_SurfaceType, context);
            TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kDiscard_ContentChangeMode);
            TestSurfaceNoCanvas(reporter, kGpuScratch_SurfaceType, context, SkSurface::kDiscard_ContentChangeMode);
            TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kRetain_ContentChangeMode);
            TestSurfaceNoCanvas(reporter, kGpuScratch_SurfaceType, context, SkSurface::kRetain_ContentChangeMode);
            TestGetTexture(reporter, kGpu_SurfaceType, context);
            TestGetTexture(reporter, kGpuScratch_SurfaceType, context);
        }
    }
#endif
}
