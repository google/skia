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
#include "GrContextPriv.h"
#include "GrResourceProvider.h"
#include "GrTextureProxy.h"

static const int DEV_W = 10, DEV_H = 10;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);
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

static void run_test(skiatest::Reporter* reporter, GrContext* context,
                     int arraySize, GrPixelConfig config) {
    SkTDArray<uint16_t> controlPixelData;
    // We will read back into an 8888 buffer since 565/4444 read backs aren't supported
    SkTDArray<GrColor> readBuffer;
    controlPixelData.setCount(arraySize);
    readBuffer.setCount(arraySize);

    for (int i = 0; i < arraySize; i += 2) {
        controlPixelData[i] = 0xFF00;
        controlPixelData[i + 1] = 0xFA62;
    }

    const SkImageInfo dstInfo = SkImageInfo::Make(DEV_W, DEV_H,
                                                  kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

    for (auto origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
        GrSurfaceDesc desc;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = DEV_W;
        desc.fHeight = DEV_H;
        desc.fConfig = config;
        desc.fOrigin = origin;

        sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                   desc, SkBudgeted::kNo,
                                                                   controlPixelData.begin(), 0);
        SkASSERT(proxy);

        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                        std::move(proxy), nullptr);

        SkAssertResult(sContext->readPixels(dstInfo, readBuffer.begin(), 0, 0, 0));

        if (kRGBA_4444_GrPixelConfig == config) {
            check_4444(reporter, controlPixelData, readBuffer);
        } else {
            SkASSERT(kRGB_565_GrPixelConfig == config);
            check_565(reporter, controlPixelData, readBuffer);
        }
    }
}

static const int CONTROL_ARRAY_SIZE = DEV_W * DEV_H;

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGBA4444TextureTest, reporter, ctxInfo) {
    run_test(reporter, ctxInfo.grContext(), CONTROL_ARRAY_SIZE, kRGBA_4444_GrPixelConfig);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RGB565TextureTest, reporter, ctxInfo) {
    run_test(reporter, ctxInfo.grContext(), CONTROL_ARRAY_SIZE, kRGB_565_GrPixelConfig);
}

#endif
