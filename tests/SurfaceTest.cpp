
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvas.h"
#include "SkRRect.h"
#include "SkSurface.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#else
class GrContextFactory;
class GrContext;
#endif

enum SurfaceType {
    kRaster_SurfaceType,
    kGpu_SurfaceType,
    kPicture_SurfaceType
};

static SkSurface* createSurface(SurfaceType surfaceType, GrContext* context) {
    static const SkImageInfo imageSpec = {
        10,  // width
        10,  // height
        kPMColor_SkColorType,
        kPremul_SkAlphaType
    };

    switch (surfaceType) {
    case kRaster_SurfaceType:
        return SkSurface::NewRaster(imageSpec);
    case kGpu_SurfaceType:
#if SK_SUPPORT_GPU
        SkASSERT(NULL != context);
        return SkSurface::NewRenderTarget(context, imageSpec);
#else
        SkASSERT(0);
#endif
    case kPicture_SurfaceType:
        return SkSurface::NewPicture(10, 10);
    }
    SkASSERT(0);
    return NULL;
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
    testBitmap.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    testBitmap.allocPixels();
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
static void Test_crbug263329(skiatest::Reporter* reporter,
                             GrContext* context) {
    // This is a regression test for crbug.com/263329
    // Bug was caused by onCopyOnWrite releasing the old surface texture
    // back to the scratch texture pool even though the texture is used
    // by and active SkImage_Gpu.
    SkAutoTUnref<SkSurface> surface1(createSurface(kGpu_SurfaceType, context));
    SkAutoTUnref<SkSurface> surface2(createSurface(kGpu_SurfaceType, context));
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
    if (surfaceType == kGpu_SurfaceType) {
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

static void TestSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    TestSurfaceCopyOnWrite(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceCopyOnWrite(reporter, kPicture_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kPicture_SurfaceType, NULL);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kDiscard_ContentChangeMode);
    TestSurfaceNoCanvas(reporter, kRaster_SurfaceType, NULL, SkSurface::kRetain_ContentChangeMode);
#if SK_SUPPORT_GPU
    TestGetTexture(reporter, kRaster_SurfaceType, NULL);
    TestGetTexture(reporter, kPicture_SurfaceType, NULL);
    if (NULL != factory) {
        GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);
        if (NULL != context) {
            Test_crbug263329(reporter, context);
            TestSurfaceCopyOnWrite(reporter, kGpu_SurfaceType, context);
            TestSurfaceWritableAfterSnapshotRelease(reporter, kGpu_SurfaceType, context);
            TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kDiscard_ContentChangeMode);
            TestSurfaceNoCanvas(reporter, kGpu_SurfaceType, context, SkSurface::kRetain_ContentChangeMode);
            TestGetTexture(reporter, kGpu_SurfaceType, context);
        }
    }
#endif
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("Surface", SurfaceTestClass, TestSurface)
