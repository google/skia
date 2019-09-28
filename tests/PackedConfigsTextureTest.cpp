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

#include "tests/Test.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrTextureProxy.h"
#include "tools/gpu/ProxyUtils.h"

static const int DEV_W = 10, DEV_H = 10;
static const uint8_t TOL = 0x4;

static void check_component(skiatest::Reporter* reporter, uint8_t control, uint8_t test) {
    uint8_t diff = 0;
    if (control >= test) {
        diff = control - test;
    } else {
        diff = test - control;
    }
    REPORTER_ASSERT(reporter, diff < TOL);
}

static uint8_t expand_value(uint8_t original, int sigBits) {
    SkASSERT(sigBits >= 4);
    uint8_t inSigBitShift = 8 - sigBits;
    uint8_t duplBitShift = sigBits - inSigBitShift;
    return (original << inSigBitShift) + (original >> duplBitShift);
}

static void check_4444(skiatest::Reporter* reporter,
                       const SkTDArray<uint16_t>& controlData,
                       const SkTDArray<uint32_t>& readBuffer) {
    for (int j = 0; j < DEV_H; ++j) {
        for (int i = 0; i < DEV_W; ++i) {
            uint16_t control = controlData[i + j * DEV_H];
            uint32_t test = readBuffer[i + j * DEV_H];

            // Test alpha component
            uint8_t ctrlComp = expand_value(control & 0xF, 4);
            uint8_t testComp = GrColorUnpackA(test);
            check_component(reporter, ctrlComp, testComp);

            // Test blue component
            ctrlComp = expand_value((control >> 4) & 0xF, 4);
            testComp = GrColorUnpackB(test);
            check_component(reporter, ctrlComp, testComp);

            // Test green component
            ctrlComp = expand_value((control >> 8) & 0xF, 4);
            testComp = GrColorUnpackG(test);
            check_component(reporter, ctrlComp, testComp);

            // Test red component
            ctrlComp = expand_value((control >> 12) & 0xF, 4);
            testComp = GrColorUnpackR(test);
            check_component(reporter, ctrlComp, testComp);
        }
    }
}

static void check_565(skiatest::Reporter* reporter,
                      const SkTDArray<uint16_t>& controlData,
                      const SkTDArray<GrColor>& readBuffer) {
    for (int j = 0; j < DEV_H; ++j) {
        for (int i = 0; i < DEV_W; ++i) {
            uint16_t control = controlData[i + j * DEV_H];
            GrColor test = readBuffer[i + j * DEV_H];
            // Test blue component (5 bit control)
            uint8_t ctrlComp = expand_value(control & 0x1F, 5);
            uint8_t testComp = GrColorUnpackB(test);
            check_component(reporter, ctrlComp, testComp);

            // Test green component (6 bit control)
            ctrlComp = expand_value((control >> 5) & 0x3F, 6);
            testComp = GrColorUnpackG(test);
            check_component(reporter, ctrlComp, testComp);

            // Test red component (5 bit control)
            ctrlComp = expand_value((control >> 11) & 0x1F, 5);
            testComp = GrColorUnpackR(test);
            check_component(reporter, ctrlComp, testComp);
        }
    }
}

static void run_test(skiatest::Reporter* reporter, GrContext* context, int arraySize,
                     SkColorType colorType) {
    SkTDArray<uint16_t> controlPixelData;
    // We will read back into an 8888 buffer since 565/4444 read backs aren't supported
    SkTDArray<GrColor> readBuffer;
    controlPixelData.setCount(arraySize);
    readBuffer.setCount(arraySize);

    for (int i = 0; i < arraySize; i += 2) {
        controlPixelData[i] = 0xF00F;
        controlPixelData[i + 1] = 0xA62F;
    }

    const SkImageInfo dstInfo =
            SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    for (auto origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
        auto proxy = sk_gpu_test::MakeTextureProxyFromData(context, GrRenderable::kNo, DEV_W, DEV_H,
                                                           colorType, kPremul_SkAlphaType, origin,
                                                           controlPixelData.begin(), 0);
        SkASSERT(proxy);

        auto sContext = context->priv().makeWrappedSurfaceContext(
                std::move(proxy), SkColorTypeToGrColorType(colorType), kPremul_SkAlphaType);

        if (!sContext->readPixels(dstInfo, readBuffer.begin(), 0, {0, 0})) {
            // We only require this to succeed if the format is renderable.
            REPORTER_ASSERT(reporter, !context->colorTypeSupportedAsSurface(colorType));
            return;
        }

        if (kARGB_4444_SkColorType == colorType) {
            check_4444(reporter, controlPixelData, readBuffer);
        } else {
            SkASSERT(kRGB_565_SkColorType == colorType);
            check_565(reporter, controlPixelData, readBuffer);
        }
    }
}

static const int CONTROL_ARRAY_SIZE = DEV_W * DEV_H;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGBA4444TextureTest, reporter, ctxInfo) {
    if (ctxInfo.grContext()->colorTypeSupportedAsImage(kARGB_4444_SkColorType)) {
        run_test(reporter, ctxInfo.grContext(), CONTROL_ARRAY_SIZE, kARGB_4444_SkColorType);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGB565TextureTest, reporter, ctxInfo) {
    if (ctxInfo.grContext()->colorTypeSupportedAsImage(kRGB_565_SkColorType)) {
        run_test(reporter, ctxInfo.grContext(), CONTROL_ARRAY_SIZE, kRGB_565_SkColorType);
    }
}
