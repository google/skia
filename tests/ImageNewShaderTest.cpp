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
    bm1.lockPixels();
    bm2.lockPixels();

    REPORTER_ASSERT(reporter, bm1.getSize() == bm2.getSize());
    REPORTER_ASSERT(reporter, 0 == memcmp(bm1.getPixels(), bm2.getPixels(), bm1.getSize()));

    bm2.unlockPixels();
    bm1.unlockPixels();
}

void runShaderTest(skiatest::Reporter* reporter, SkSurface* source, SkSurface* destination, SkImageInfo& info) {
    SkCanvas* rasterCanvas = source->getCanvas();
    rasterCanvas->drawColor(0xFFDEDEDE, SkXfermode::kSrc_Mode);

    SkAutoTUnref<SkImage> rasterImage(source->newImageSnapshot());
    SkAutoTUnref<SkShader> rasterShader(rasterImage->newShader(
            SkShader::kRepeat_TileMode,
            SkShader::kRepeat_TileMode));

    SkPaint paint;
    paint.setShader(rasterShader);
    SkCanvas* canvasDest = destination->getCanvas();
    canvasDest->clear(SK_ColorTRANSPARENT);
    canvasDest->drawPaint(paint);

    SkIRect rect = SkIRect::MakeXYWH(0, 0, 5, 5);

    SkBitmap bmOrig;
    rasterCanvas->readPixels(rect, &bmOrig);

    SkBitmap bm;
    canvasDest->readPixels(rect, &bm);

    testBitmapEquality(reporter, bmOrig, bm);
}

DEF_TEST(ImageNewShader, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> srcSurface(SkSurface::NewRaster(info));
    SkAutoTUnref<SkSurface> dstSurface(SkSurface::NewRaster(info));

    runShaderTest(reporter, srcSurface.get(), dstSurface.get(), info);
}

#if SK_SUPPORT_GPU

void gpuToGpu(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> srcSurface(SkSurface::NewRenderTarget(context, info));
    SkAutoTUnref<SkSurface> dstSurface(SkSurface::NewRenderTarget(context, info));

    runShaderTest(reporter, srcSurface.get(), dstSurface.get(), info);
}

void gpuToRaster(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> srcSurface(SkSurface::NewRenderTarget(context, info));
    SkAutoTUnref<SkSurface> dstSurface(SkSurface::NewRaster(info));

    runShaderTest(reporter, srcSurface.get(), dstSurface.get(), info);
}

void rasterToGpu(skiatest::Reporter* reporter, GrContext* context) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(5, 5);

    SkAutoTUnref<SkSurface> srcSurface(SkSurface::NewRaster(info));
    SkAutoTUnref<SkSurface> dstSurface(SkSurface::NewRenderTarget(context, info));

    runShaderTest(reporter, srcSurface.get(), dstSurface.get(), info);
}

DEF_GPUTEST(ImageNewShader_GPU, reporter, factory) {
    GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);

    // GPU -> GPU
    gpuToGpu(reporter, context);

    // GPU -> RASTER
    gpuToRaster(reporter, context);


    // RASTER -> GPU
    rasterToGpu(reporter, context);
}

#endif
