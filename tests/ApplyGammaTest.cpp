/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <initializer_list>
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"

#include "SkCanvas.h"
#include "SkGammaColorFilter.h"
#include "SkSurface.h"
#include "SkUtils.h"

 // using anonymous namespace because these functions are used as template params.
namespace {
/** convert 0..1 linear value to 0..1 srgb */
float linear_to_srgb(float linear) {
    if (linear <= 0.0031308) {
        return linear * 12.92f;
    } else {
        return 1.055f * powf(linear, 1.f / 2.4f) - 0.055f;
    }
}
}

bool check_gamma(uint32_t src, uint32_t dst, float gamma, float error, uint32_t* expected) {
    if (SkScalarNearlyEqual(gamma, 1.f)) {
        *expected = src;
        return src == dst;
    } else {
        bool result = true;
        uint32_t expectedColor = src & 0xff000000;

        // Alpha should always be exactly preserved.
        if ((src & 0xff000000) != (dst & 0xff000000)) {
            result = false;
        }

        for (int c = 0; c < 3; ++c) {
            uint8_t srcComponent = (src & (0xff << (c * 8))) >> (c * 8);
            float lower = SkTMax(0.f, (float)srcComponent - error);
            float upper = SkTMin(255.f, (float)srcComponent + error);
            if (SkScalarNearlyEqual(gamma, 1.0f / 2.2f)) {
                lower = linear_to_srgb(lower / 255.f);
                upper = linear_to_srgb(upper / 255.f);
            } else {
                lower = powf(lower / 255.f, gamma);
                upper = powf(upper / 255.f, gamma);
            }
            SkASSERT(lower >= 0.f && lower <= 255.f);
            SkASSERT(upper >= 0.f && upper <= 255.f);
            uint8_t dstComponent = (dst & (0xff << (c * 8))) >> (c * 8);
            if (dstComponent < SkScalarFloorToInt(lower * 255.f) ||
                dstComponent > SkScalarCeilToInt(upper * 255.f)) {
                result = false;
            }
            uint8_t expectedComponent = SkScalarRoundToInt((lower + upper) * 127.5f);
            expectedColor |= expectedComponent << (c * 8);
        }

        *expected = expectedColor;
        return result;
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ApplyGamma, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    static const int kW = 10;
    static const int kH = 10;
    static const size_t kRowBytes = sizeof(uint32_t) * kW;

    GrSurfaceDesc baseDesc;
    baseDesc.fConfig = kRGBA_8888_GrPixelConfig;
    baseDesc.fWidth = kW;
    baseDesc.fHeight = kH;

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(kW, kH);

    SkAutoTMalloc<uint32_t> srcPixels(kW * kH);
    for (int i = 0; i < kW * kH; ++i) {
        srcPixels.get()[i] = i;
    }

    SkBitmap bm;
    bm.installPixels(ii, srcPixels.get(), kRowBytes);

    SkAutoTMalloc<uint32_t> read(kW * kH);

    // We allow more error on GPUs with lower precision shader variables.
    float error = context->caps()->shaderCaps()->floatPrecisionVaries() ? 1.2f : 0.5f;

    for (auto dOrigin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto gamma : { 1.0f, 1.0f / 1.8f, 1.0f / 2.2f }) {
            sk_sp<SkSurface> dst(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, 
                                                             ii, 0, dOrigin, nullptr));

            if (!dst) {
                ERRORF(reporter, "Could not create surfaces for copy surface test.");
                continue;
            }

            SkCanvas* dstCanvas = dst->getCanvas();

            dstCanvas->clear(SK_ColorRED);
            dstCanvas->flush();

            SkPaint gammaPaint;
            gammaPaint.setBlendMode(SkBlendMode::kSrc);
            gammaPaint.setColorFilter(SkGammaColorFilter::Make(gamma));

            dstCanvas->drawBitmap(bm, 0, 0, &gammaPaint);
            dstCanvas->flush();

            sk_memset32(read.get(), 0, kW * kH);
            if (!dstCanvas->readPixels(ii, read.get(), kRowBytes, 0, 0)) {
                ERRORF(reporter, "Error calling readPixels");
                continue;
            }

            bool abort = false;
            // Validate that pixels were copied/transformed correctly.
            for (int y = 0; y < kH && !abort; ++y) {
                for (int x = 0; x < kW && !abort; ++x) {
                    uint32_t r = read.get()[y * kW + x];
                    uint32_t s = srcPixels.get()[y * kW + x];
                    uint32_t expected;
                    if (!check_gamma(s, r, gamma, error, &expected)) {
                        ERRORF(reporter, "Expected dst %d,%d to contain 0x%08x "
                                "from src 0x%08x and gamma %f. Got %08x",
                                x, y, expected, s, gamma, r);
                        abort = true;
                        break;
                    }
                }
            }
        }
    }
}
#endif
