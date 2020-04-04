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

#include "tests/Test.h"

#include "include/gpu/GrContext.h"
#include "include/private/SkHalf.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrTextureProxy.h"
#include "tools/gpu/ProxyUtils.h"

#include <float.h>

static const int DEV_W = 100, DEV_H = 100;

template <typename T>
void runFPTest(skiatest::Reporter* reporter, GrContext* context, T min, T max, T epsilon, T maxInt,
               int arraySize, GrColorType colorType) {
    if (0 != arraySize % 4) {
        REPORT_FAILURE(reporter, "(0 != arraySize % 4)",
                       SkString("arraySize must be divisible by 4."));
        return;
    }

    SkTDArray<T> controlPixelData, readBuffer;
    controlPixelData.setCount(arraySize);
    readBuffer.setCount(arraySize);

    for (int i = 0; i < arraySize; i += 4) {
        controlPixelData[i + 0] = min;
        controlPixelData[i + 1] = max;
        controlPixelData[i + 2] = epsilon;
        controlPixelData[i + 3] = maxInt;
    }

    for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto fpProxy = sk_gpu_test::MakeTextureProxyFromData(
                context, GrRenderable::kYes, origin,
                {colorType, kPremul_SkAlphaType, nullptr, DEV_W, DEV_H},
                controlPixelData.begin(), 0);
        // Floating point textures are NOT supported everywhere
        if (!fpProxy) {
            continue;
        }

        auto sContext = context->priv().makeWrappedSurfaceContext(std::move(fpProxy), colorType,
                                                                  kPremul_SkAlphaType);
        REPORTER_ASSERT(reporter, sContext);

        bool result = sContext->readPixels({colorType, kPremul_SkAlphaType, nullptr, DEV_W, DEV_H},
                                           readBuffer.begin(), 0, {0, 0}, context);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter,
                        0 == memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.bytes()));
    }
}

static const int HALF_ALPHA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 1 /*alpha-only*/;
static const SkHalf kMaxIntegerRepresentableInHalfFloatingPoint = 0x6800;  // 2 ^ 11

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatAlphaTextureTest, reporter, ctxInfo) {
    runFPTest<SkHalf>(reporter, ctxInfo.grContext(), SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_ALPHA_CONTROL_ARRAY_SIZE,
                      GrColorType::kAlpha_F16);
}

static const int HALF_RGBA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4 /*RGBA*/;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatRGBATextureTest, reporter, ctxInfo) {
    runFPTest<SkHalf>(reporter, ctxInfo.grContext(), SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_RGBA_CONTROL_ARRAY_SIZE,
                      GrColorType::kRGBA_F16);
}
