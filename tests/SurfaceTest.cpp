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

static void release_storage(void* pixels, void* context) {
    SkASSERT(pixels == context);
    sk_free(pixels);
}

static SkSurface* createSurface(SurfaceType surfaceType, GrContext* context,
                                SkImageInfo* requestedInfo = NULL) {
    static const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);

    if (requestedInfo) {
        *requestedInfo = info;
    }

    switch (surfaceType) {
        case kRaster_SurfaceType:
            return SkSurface::NewRaster(info);
        case kRasterDirect_SurfaceType: {
            const size_t rowBytes = info.minRowBytes();
            void* storage = sk_malloc_throw(info.getSafeSize(rowBytes));
            return SkSurface::NewRasterDirectReleaseProc(info, storage, rowBytes,
                                                         release_storage, storage);
        }
        case kGpu_SurfaceType:
            return SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0, NULL);
        case kGpuScratch_SurfaceType:
            return SkSurface::NewRenderTarget(context, SkSurface::kYes_Budgeted, info, 0, NULL);
    }
    return NULL;
}

enum ImageType {
    kRasterCopy_ImageType,
    kRasterData_ImageType,
    kGpu_ImageType,
    kCodec_ImageType,
};

#include "SkImageGenerator.h"

class EmptyGenerator : public SkImageGenerator {
protected:
    bool onGetInfo(SkImageInfo* info) SK_OVERRIDE {
        *info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
        return true;
    }
};

static void test_empty_image(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    
    REPORTER_ASSERT(reporter, NULL == SkImage::NewRasterCopy(info, NULL, 0));
    REPORTER_ASSERT(reporter, NULL == SkImage::NewRasterData(info, NULL, 0));
    REPORTER_ASSERT(reporter, NULL == SkImage::NewFromGenerator(SkNEW(EmptyGenerator)));
}

static void test_empty_surface(skiatest::Reporter* reporter, GrContext* ctx) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    
    REPORTER_ASSERT(reporter, NULL == SkSurface::NewRaster(info));
    REPORTER_ASSERT(reporter, NULL == SkSurface::NewRasterDirect(info, NULL, 0));
    if (ctx) {
        REPORTER_ASSERT(reporter, NULL ==
                        SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info, 0, NULL));
    }
}

static void test_image(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    SkData* data = SkData::NewUninitialized(size);

    REPORTER_ASSERT(reporter, data->unique());
    SkImage* image = SkImage::NewRasterData(info, data, rowBytes);
    REPORTER_ASSERT(reporter, !data->unique());
    image->unref();
    REPORTER_ASSERT(reporter, data->unique());
    data->unref();
}

static SkImage* createImage(ImageType imageType, GrContext* context, SkColor color) {
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    const size_t rowBytes = info.minRowBytes();
    const size_t size = rowBytes * info.height();

    SkAutoTUnref<SkData> data(SkData::NewUninitialized(size));
    void* addr = data->writable_data();
    sk_memset32((SkPMColor*)addr, pmcolor, SkToInt(size >> 2));

    switch (imageType) {
        case kRasterCopy_ImageType:
            return SkImage::NewRasterCopy(info, addr, rowBytes);
        case kRasterData_ImageType:
            return SkImage::NewRasterData(info, data, rowBytes);
        case kGpu_ImageType: {
            SkAutoTUnref<SkSurface> surf(
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0));
            surf->getCanvas()->clear(color);
            return surf->newImageSnapshot();
        }
        case kCodec_ImageType: {
            SkBitmap bitmap;
            bitmap.installPixels(info, addr, rowBytes);
            SkAutoTUnref<SkData> src(
                 SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
            return SkImage::NewFromData(src);
        }
    }
    SkASSERT(false);
    return NULL;
}

static void set_pixels(SkPMColor pixels[], int count, SkPMColor color) {
    sk_memset32(pixels, color, count);
}
static bool has_pixels(const SkPMColor pixels[], int count, SkPMColor expected) {
    for (int i = 0; i < count; ++i) {
        if (pixels[i] != expected) {
            return false;
        }
    }
    return true;
}

static void test_image_readpixels(skiatest::Reporter* reporter, SkImage* image,
                                  SkPMColor expected) {
    const SkPMColor notExpected = ~expected;

    const int w = 2, h = 2;
    const size_t rowBytes = w * sizeof(SkPMColor);
    SkPMColor pixels[w*h];

    SkImageInfo info;

    info = SkImageInfo::MakeUnknown(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, 0));

    // out-of-bounds should fail
    info = SkImageInfo::MakeN32Premul(w, h);
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, -w, 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, -h));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, image->width(), 0));
    REPORTER_ASSERT(reporter, !image->readPixels(info, pixels, rowBytes, 0, image->height()));

    // top-left should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, 0, 0));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - w, image->height() - h));
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h, expected));

    // partial top-left should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes, -1, -1));
    REPORTER_ASSERT(reporter, pixels[3] == expected);
    REPORTER_ASSERT(reporter, has_pixels(pixels, w*h - 1, notExpected));

    // partial bottom-right should succeed
    set_pixels(pixels, w*h, notExpected);
    REPORTER_ASSERT(reporter, image->readPixels(info, pixels, rowBytes,
                                                image->width() - 1, image->height() - 1));
    REPORTER_ASSERT(reporter, pixels[0] == expected);
    REPORTER_ASSERT(reporter, has_pixels(&pixels[1], w*h - 1, notExpected));
}

static void test_imagepeek(skiatest::Reporter* reporter, GrContextFactory* factory) {
    static const struct {
        ImageType   fType;
        bool        fPeekShouldSucceed;
        const char* fName;
    } gRec[] = {
        { kRasterCopy_ImageType,    true,       "RasterCopy"    },
        { kRasterData_ImageType,    true,       "RasterData"    },
        { kGpu_ImageType,           false,      "Gpu"           },
        { kCodec_ImageType,         false,      "Codec"         },
    };

    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);

    GrContext* ctx = NULL;
#if SK_SUPPORT_GPU
    ctx = factory->get(GrContextFactory::kNative_GLContextType);
#endif

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkImageInfo info;
        size_t rowBytes;

        SkAutoTUnref<SkImage> image(createImage(gRec[i].fType, ctx, color));
        if (!image.get()) {
            SkDebugf("failed to createImage[%d] %s\n", i, gRec[i].fName);
            continue;   // gpu may not be enabled
        }
        const void* addr = image->peekPixels(&info, &rowBytes);
        bool success = SkToBool(addr);
        REPORTER_ASSERT(reporter, gRec[i].fPeekShouldSucceed == success);
        if (success) {
            REPORTER_ASSERT(reporter, 10 == info.width());
            REPORTER_ASSERT(reporter, 10 == info.height());
            REPORTER_ASSERT(reporter, kN32_SkColorType == info.colorType());
            REPORTER_ASSERT(reporter, kPremul_SkAlphaType == info.alphaType() ||
                            kOpaque_SkAlphaType == info.alphaType());
            REPORTER_ASSERT(reporter, info.minRowBytes() <= rowBytes);
            REPORTER_ASSERT(reporter, pmcolor == *(const SkPMColor*)addr);
        }

        test_image_readpixels(reporter, image, pmcolor);
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

    int cnt;
#if SK_SUPPORT_GPU
    cnt = GrContextFactory::kGLContextTypeCnt;
#else
    cnt = 1;
#endif

    for (int i= 0; i < cnt; ++i) {
        GrContext* context = NULL;
#if SK_SUPPORT_GPU
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }
        context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }
#endif
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            SkImageInfo info, requestInfo;
            size_t rowBytes;

            SkAutoTUnref<SkSurface> surface(createSurface(gRec[i].fType, context,
                                                          &requestInfo));
            surface->getCanvas()->clear(color);

            const void* addr = surface->getCanvas()->peekPixels(&info, &rowBytes);
            bool success = SkToBool(addr);
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
        REPORTER_ASSERT(reporter, texture);
        REPORTER_ASSERT(reporter, 0 != texture->getTextureHandle());
    } else {
        REPORTER_ASSERT(reporter, NULL == texture);
    }
    surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
    REPORTER_ASSERT(reporter, image->getTexture() == texture);
}

#include "GrGpuResourcePriv.h"
#include "SkGpuDevice.h"
#include "SkImage_Gpu.h"
#include "SkSurface_Gpu.h"

SkSurface::Budgeted is_budgeted(SkSurface* surf) {
    return ((SkSurface_Gpu*)surf)->getDevice()->accessRenderTarget()->resourcePriv().isBudgeted() ?
        SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
}

SkSurface::Budgeted is_budgeted(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture()->resourcePriv().isBudgeted() ?
        SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
}

static void test_surface_budget(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(8,8);
    for (int i = 0; i < 2; ++i) {
        SkSurface::Budgeted sbudgeted = i ? SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
        for (int j = 0; j < 2; ++j) {
            SkSurface::Budgeted ibudgeted = j ? SkSurface::kYes_Budgeted : SkSurface::kNo_Budgeted;
            SkAutoTUnref<SkSurface>
                surface(SkSurface::NewRenderTarget(context, sbudgeted, info, 0));
            SkASSERT(surface);
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));

            SkAutoTUnref<SkImage> image(surface->newImageSnapshot(ibudgeted));

            // Initially the image shares a texture with the surface, and the surface decides
            // whether it is budgeted or not.
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(image));

            // Now trigger copy-on-write
            surface->getCanvas()->clear(SK_ColorBLUE);

            // They don't share a texture anymore. They should each have made their own budget
            // decision.
            REPORTER_ASSERT(reporter, sbudgeted == is_budgeted(surface));
            REPORTER_ASSERT(reporter, ibudgeted == is_budgeted(image));
        }
    }
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

    test_empty_image(reporter);
    test_empty_surface(reporter, NULL);

    test_imagepeek(reporter, factory);
    test_canvaspeek(reporter, factory);

#if SK_SUPPORT_GPU
    TestGetTexture(reporter, kRaster_SurfaceType, NULL);
    if (factory) {
        for (int i= 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
            GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
            if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
                continue;
            }
            GrContext* context = factory->get(glCtxType);
            if (context) {
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
                test_empty_surface(reporter, context);
                test_surface_budget(reporter, context);
            }
        }
    }
#endif
}
