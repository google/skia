/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This is a straightforward test of floating point textures, which are
 * supported on some platforms.  As of right now, this test only supports
 * 32 bit floating point textures, and indeed floating point test values
 * have been selected to require 32 bits of precision and full IEEE conformance
 */

#include <float.h>
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "SkHalf.h"

static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);

template <typename T>
void runFPTest(skiatest::Reporter* reporter, GrContext* context,
               T min, T max, T epsilon, T maxInt, int arraySize, GrPixelConfig config) {
    SkTDArray<T> controlPixelData, readBuffer;
    controlPixelData.setCount(arraySize);
    readBuffer.setCount(arraySize);

    for (int i = 0; i < arraySize; i += 4) {
        controlPixelData[i + 0] = min;
        controlPixelData[i + 1] = max;
        controlPixelData[i + 2] = epsilon;
        controlPixelData[i + 3] = maxInt;
    }

    for (int origin = 0; origin < 2; ++origin) {
        GrSurfaceDesc desc;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fWidth = DEV_W;
        desc.fHeight = DEV_H;
        desc.fConfig = config;
        desc.fOrigin = 0 == origin ? kTopLeft_GrSurfaceOrigin : kBottomLeft_GrSurfaceOrigin;
        sk_sp<GrTexture> fpTexture(context->textureProvider()->createTexture(
            desc, SkBudgeted::kNo, controlPixelData.begin(), 0));
        // Floating point textures are NOT supported everywhere
        if (nullptr == fpTexture) {
            continue;
        }
        fpTexture->readPixels(0, 0, DEV_W, DEV_H, desc.fConfig, readBuffer.begin(), 0);
        REPORTER_ASSERT(reporter,
                        0 == memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.bytes()));
    }
}

static const int FP_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4/*RGBA*/;
static const float kMaxIntegerRepresentableInSPFloatingPoint = 16777216;  // 2 ^ 24

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FloatingPointTextureTest, reporter, ctxInfo) {
    runFPTest<float>(reporter, ctxInfo.grContext(), FLT_MIN, FLT_MAX, FLT_EPSILON,
                     kMaxIntegerRepresentableInSPFloatingPoint,
                     FP_CONTROL_ARRAY_SIZE, kRGBA_float_GrPixelConfig);
}

static const int HALF_ALPHA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 1 /*alpha-only*/;
static const SkHalf kMaxIntegerRepresentableInHalfFloatingPoint = 0x6800;  // 2 ^ 11

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatAlphaTextureTest, reporter, ctxInfo) {
    runFPTest<SkHalf>(reporter, ctxInfo.grContext(), SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
        kMaxIntegerRepresentableInHalfFloatingPoint,
        HALF_ALPHA_CONTROL_ARRAY_SIZE, kAlpha_half_GrPixelConfig);
}

static const int HALF_RGBA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4 /*RGBA*/;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatRGBATextureTest, reporter, ctxInfo) {
    runFPTest<SkHalf>(reporter, ctxInfo.grContext(), SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
        kMaxIntegerRepresentableInHalfFloatingPoint,
        HALF_RGBA_CONTROL_ARRAY_SIZE, kRGBA_half_GrPixelConfig);
}

#endif
