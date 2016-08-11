/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This is a straightforward test of using packed pixel configs (4444, 565).
 * This test will make sure that these RGBA_4444 and RGB_565 are always supported
 * as valid texturing configs.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"

static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);

template <typename T>
void runTest(skiatest::Reporter* reporter, GrContext* context,
             T val1, T val2, int arraySize, GrPixelConfig config) {
    SkTDArray<T> controlPixelData, readBuffer;
    controlPixelData.setCount(arraySize);
    readBuffer.setCount(arraySize);

    for (int i = 0; i < arraySize; i += 2) {
        controlPixelData[i] = val1;
        controlPixelData[i + 1] = val2;
    }

    for (int origin = 0; origin < 2; ++origin) {
        GrSurfaceDesc desc;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = DEV_W;
        desc.fHeight = DEV_H;
        desc.fConfig = config;
        desc.fOrigin = 0 == origin ?
            kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
        SkAutoTUnref<GrTexture> fpTexture(context->textureProvider()->createTexture(
            desc, SkBudgeted::kNo, controlPixelData.begin(), 0));
        SkASSERT(fpTexture);
        fpTexture->readPixels(0, 0, DEV_W, DEV_H, desc.fConfig, readBuffer.begin(), 0);
        REPORTER_ASSERT(reporter,
                        0 == memcmp(readBuffer.begin(), controlPixelData.begin(),
                                    readBuffer.bytes()));
    }
}

static const int CONTROL_ARRAY_SIZE = DEV_W * DEV_H;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGBA4444TextureTest, reporter, ctxInfo) {
    runTest<uint16_t>(reporter, ctxInfo.grContext(), 0xFF00, 0xFA62,
                      CONTROL_ARRAY_SIZE, kRGBA_4444_GrPixelConfig);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGB565TextureTest, reporter, ctxInfo) {
    runTest<uint16_t>(reporter, ctxInfo.grContext(), 0xFF00, 0xFA62,
                      CONTROL_ARRAY_SIZE, kRGB_565_GrPixelConfig);
}

#endif
