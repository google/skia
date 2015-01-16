/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkShader.h"
#include "SkSurface.h"

#include "Test.h"

void testBitmapEquality(skiatest::Reporter* reporter, SkBitmap& bm1, SkBitmap& bm2) {
    SkAutoLockPixels lockBm1(bm1);
    SkAutoLockPixels lockBm2(bm2);

    REPORTER_ASSERT(reporter, bm1.getSize() == bm2.getSize());
    REPORTER_ASSERT(reporter, 0 == memcmp(bm1.getPixels(), bm2.getPixels(), bm1.getSize()));
}

void paintSource(SkSurface* sourceSurface) {
    SkCanvas* sourceCanvas = sourceSurface->getCanvas();
    sourceCanvas->clear(0xFFDEDEDE);

    SkPaint paintColor;
    paintColor.setColor(0xFFFF0000);
    paintColor.setStyle(SkPaint::kFill_Style);

    SkRect rect = SkRect::MakeXYWH(
            SkIntToScalar(1),
            SkIntToScalar(0),
            SkIntToScalar(1),
            SkIntToScalar(sourceSurface->height()));

    sourceCanvas->drawRect(rect, paintColor);
}

void runShaderTest(skiatest::Reporter* reporter, SkSurface* sourceSurface, SkSurface* destinationSurface, SkImageInfo& info) {
    paintSource(sourceSurface);

    SkAutoTUnref<SkImage> sourceImage(sourceSurface->newImageSnapshot());
    SkAutoTUnref<SkShader> sourceShader(sourceImage->newShader(
            SkShader::kRepeat_TileMode,
            SkShader::kRepeat_TileMode));

    SkPaint paint;
    paint.setShader(sourceShader);

    SkCanvas* destinationCanvas = destinationSurface->getCanvas();
    destinationCanvas->clear(SK_ColorTRANSPARENT);
    destinationCanvas->drawPaint(paint);

    SkIRect rect = info.bounds();

    SkBitmap bmOrig;
    sourceSurface->getCanvas()->readPixels(rect, &bmOrig);


    SkBitmap bm;
    destinationCanvas->readPixels(rect, &bm);

    testBitmapEquality(reporter, bmOrig, bm);



    // Test with a translated shader
    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-1), SkIntToScalar(0));

    SkAutoTUnref<SkShader> sourceShaderTranslated(sourceImage->newShader(
            SkShader::kRepeat_TileMode,
            SkShader::kRepeat_TileMode,
            &matrix));

    destinationCanvas->clear(SK_ColorTRANSPARENT);

    SkPaint paintTranslated;
    paintTranslated.setShader(sourceShaderTranslated);

    destinationCanvas->drawPaint(paintTranslated);

    SkBitmap bmt;
    destinationCanvas->readPixels(rect, &bmt);

    //  Test correctness
    {
        SkAutoLockPixels lockBm(bmt);
        for (int y = 0; y < info.height(); y++) {
            REPORTER_ASSERT(reporter, 0xFFFF0000 == bmt.getColor(0, y));

            for (int x = 1; x < info.width(); x++) {
                REPORTER_ASSERT(reporter, 0xFFDEDEDE == bmt.getColor(x, y));
            }
        }
    }
}

DEF_TEST(ImageNewShader, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> sourceSurface(SkSurface::NewRaster(info));
    SkAutoTUnref<SkSurface> destinationSurface(SkSurface::NewRaster(info));

    runShaderTest(reporter, sourceSurface.get(), destinationSurface.get(), info);
}

#if SK_SUPPORT_GPU

void gpuToGpu(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> sourceSurface(
        SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info));
    SkAutoTUnref<SkSurface> destinationSurface(
        SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info));

    runShaderTest(reporter, sourceSurface.get(), destinationSurface.get(), info);
}

void gpuToRaster(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> sourceSurface(SkSurface::NewRenderTarget(context,
        SkSurface::kNo_Budgeted, info));
    SkAutoTUnref<SkSurface> destinationSurface(SkSurface::NewRaster(info));

    runShaderTest(reporter, sourceSurface.get(), destinationSurface.get(), info);
}

void rasterToGpu(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> sourceSurface(SkSurface::NewRaster(info));
    SkAutoTUnref<SkSurface> destinationSurface(SkSurface::NewRenderTarget(context,
        SkSurface::kNo_Budgeted, info));

    runShaderTest(reporter, sourceSurface.get(), destinationSurface.get(), info);
}

DEF_GPUTEST(ImageNewShader_GPU, reporter, factory) {
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;

        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }

        GrContext* context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }

        //  GPU -> GPU
        gpuToGpu(reporter, context);

        //  GPU -> RASTER
        gpuToRaster(reporter, context);

        //  RASTER -> GPU
        rasterToGpu(reporter, context);
    }
}

#endif
