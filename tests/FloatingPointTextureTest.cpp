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

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkHalf.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ProxyUtils.h"

#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>

struct GrContextOptions;

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
    controlPixelData.resize(arraySize);
    readBuffer.resize(arraySize);

    for (int i = 0; i < arraySize; i += 4) {
        controlPixelData[i + 0] = min;
        controlPixelData[i + 1] = max;
        controlPixelData[i + 2] = epsilon;
        controlPixelData[i + 3] = maxInt;
    }

    for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        GrImageInfo info(colorType, kPremul_SkAlphaType, nullptr, {DEV_W, DEV_H});
        GrCPixmap controlPixmap(info, controlPixelData.begin(), info.minRowBytes());
        auto fpView = sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                                GrRenderable::kYes,
                                                                origin,
                                                                controlPixmap);
        // Floating point textures are NOT supported everywhere
        if (!fpView) {
            continue;
        }

        auto sc = dContext->priv().makeSC(std::move(fpView), info.colorInfo());
        REPORTER_ASSERT(reporter, sc);

        GrPixmap readPixmap(info, readBuffer.begin(), info.minRowBytes());
        bool result = sc->readPixels(dContext, readPixmap, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter,
            !memcmp(readBuffer.begin(), controlPixelData.begin(), readBuffer.size_bytes()));
    }
}

static const int HALF_ALPHA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 1 /*alpha-only*/;
static const SkHalf kMaxIntegerRepresentableInHalfFloatingPoint = 0x6800;  // 2 ^ 11

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(HalfFloatAlphaTextureTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();

    runFPTest<SkHalf>(reporter, direct, SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_ALPHA_CONTROL_ARRAY_SIZE,
                      GrColorType::kAlpha_F16);
}

static const int HALF_RGBA_CONTROL_ARRAY_SIZE = DEV_W * DEV_H * 4 /*RGBA*/;

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(HalfFloatRGBATextureTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();

    runFPTest<SkHalf>(reporter, direct, SK_HalfMin, SK_HalfMax, SK_HalfEpsilon,
                      kMaxIntegerRepresentableInHalfFloatingPoint, HALF_RGBA_CONTROL_ARRAY_SIZE,
                      GrColorType::kRGBA_F16);
}
