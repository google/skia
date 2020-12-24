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

#include "include/gpu/GrDirectContext.h"
#include "include/private/SkHalf.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "tools/gpu/ProxyUtils.h"

#include <float.h>

static const int DEV_W = 100, DEV_H = 100;

template <typename T>
void runFPTest(skiatest::Reporter* reporter, GrDirectContext* dContext,
               T min, T max, T epsilon, T maxInt,
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
        GrImageInfo info(colorType, kPremul_SkAlphaType, nullptr, {DEV_W, DEV_H});
        GrPixmap controlPixmap(info, controlPixelData.begin(), info.minRowBytes());
        auto fpView = sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                                GrRenderable::kYes,
                                                                origin,
                                                                controlPixmap);
        // Floating point textures are NOT supported everywhere
        if (!fpView) {
            continue;
        }

        auto sContext = GrSurfaceContext::Make(dContext, std::move(fpView), info.colorInfo());
        REPORTER_ASSERT(reporter, sContext);

        GrPixmap readPixmap(info, readBuffer.begin(), info.minRowBytes());
        bool result = sContext->readPixels(dContext, readPixmap, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter,
                        !memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.bytes()));
    }
}

static const int HALF_ALPHA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 1 /*alpha-only*/;
static const SkHalf kMaxIntegerRepresentableInHalfFloatingPoint = 0x6800;  // 2 ^ 11

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatAlphaTextureTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();

    runFPTest<SkHalf>(reporter, direct, SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_ALPHA_CONTROL_ARRAY_SIZE,
                      GrColorType::kAlpha_F16);
}

static const int HALF_RGBA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4 /*RGBA*/;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(HalfFloatRGBATextureTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();

    runFPTest<SkHalf>(reporter, direct, SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_RGBA_CONTROL_ARRAY_SIZE,
                      GrColorType::kRGBA_F16);
}
