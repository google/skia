
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
    static const SkImage::Info imageSpec = {
        10,  // width
        10,  // height
        SkImage::kPMColor_ColorType,
        SkImage::kPremul_AlphaType
    };

    switch (surfaceType) {
    case kRaster_SurfaceType:
        return SkSurface::NewRaster(imageSpec);
    case kGpu_SurfaceType:
        SkASSERT(NULL != context);
        return SkSurface::NewRenderTarget(context, imageSpec);
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
        SkImage* imageBefore = surface->newImageShapshot();         \
        SkAutoTUnref<SkImage> aur_before(imageBefore);              \
        canvas-> command ;                                          \
        SkImage* imageAfter = surface->newImageShapshot();          \
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
    SkSurface* surface = createSurface(surfaceType, context);
    SkAutoTUnref<SkSurface> aur_surface(surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(1);
    surface->newImageShapshot()->unref();  // Create and destroy SkImage
    canvas->clear(2);
}

static void TestSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    TestSurfaceCopyOnWrite(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceCopyOnWrite(reporter, kPicture_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kRaster_SurfaceType, NULL);
    TestSurfaceWritableAfterSnapshotRelease(reporter, kPicture_SurfaceType, NULL);
#if SK_SUPPORT_GPU
    if (NULL != factory) {
        GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);
        TestSurfaceCopyOnWrite(reporter, kGpu_SurfaceType, context);
        TestSurfaceWritableAfterSnapshotRelease(reporter, kGpu_SurfaceType, context);
    }
#endif
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("Surface", SurfaceTestClass, TestSurface)
