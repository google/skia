/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrShaderCaps.h"
#include "GrTypes.h"
#include "SkBitmap.h"
#include "SkBlendMode.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkImageInfo.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "Test.h"

#include <math.h>

/** convert 0..1 linear value to 0..1 srgb */
static float linear_to_srgb(float linear) {
    if (linear <= 0.0031308) {
        return linear * 12.92f;
    } else {
        return 1.055f * powf(linear, 1.f / 2.4f) - 0.055f;
    }
}

/** convert 0..1 srgb value to 0..1 linear */
static float srgb_to_linear(float srgb) {
    if (srgb <= 0.04045f) {
        return srgb / 12.92f;
    } else {
        return powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
}

bool check_gamma(uint32_t src, uint32_t dst, bool toSRGB, float error,
                 uint32_t* expected) {
    bool result = true;
    uint32_t expectedColor = src & 0xff000000;

    // Alpha should always be exactly preserved.
    if ((src & 0xff000000) != (dst & 0xff000000)) {
        result = false;
    }

    // need to unpremul before we can perform srgb magic
    float invScale = 0;
    float alpha = SkGetPackedA32(src);
    if (alpha) {
        invScale = 255.0f / alpha;
    }

    for (int c = 0; c < 3; ++c) {
        float srcComponent = ((src & (0xff << (c * 8))) >> (c * 8)) * invScale;
        float lower = SkTMax(0.f, srcComponent - error);
        float upper = SkTMin(255.f, srcComponent + error);
        if (toSRGB) {
            lower = linear_to_srgb(lower / 255.f);
            upper = linear_to_srgb(upper / 255.f);
        } else {
            lower = srgb_to_linear(lower / 255.f);
            upper = srgb_to_linear(upper / 255.f);
        }
        lower *= alpha;
        upper *= alpha;
        SkASSERT(lower >= 0.f && lower <= 255.f);
        SkASSERT(upper >= 0.f && upper <= 255.f);
        uint8_t dstComponent = (dst & (0xff << (c * 8))) >> (c * 8);
        if (dstComponent < SkScalarFloorToInt(lower) ||
            dstComponent > SkScalarCeilToInt(upper)) {
            result = false;
        }
        uint8_t expectedComponent = SkScalarRoundToInt((lower + upper) * 0.5f);
        expectedColor |= expectedComponent << (c * 8);
    }

    *expected = expectedColor;
    return result;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ApplyGamma, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    static const int kW = 256;
    static const int kH = 256;
    static const size_t kRowBytes = sizeof(uint32_t) * kW;

    GrSurfaceDesc baseDesc;
    baseDesc.fConfig = kRGBA_8888_GrPixelConfig;
    baseDesc.fWidth = kW;
    baseDesc.fHeight = kH;

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(kW, kH);

    SkAutoTMalloc<uint32_t> srcPixels(kW * kH);
    for (int y = 0; y < kH; ++y) {
        for (int x = 0; x < kW; ++x) {
            srcPixels.get()[y*kW+x] = SkPreMultiplyARGB(x, y, x, 0xFF);
        }
    }

    SkBitmap bm;
    bm.installPixels(ii, srcPixels.get(), kRowBytes);

    SkAutoTMalloc<uint32_t> read(kW * kH);

    // We allow more error on GPUs with lower precision shader variables.
    float error = context->priv().caps()->shaderCaps()->halfIs32Bits() ? 0.5f : 1.2f;

    for (auto toSRGB : { false, true }) {
        sk_sp<SkSurface> dst(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii));

        if (!dst) {
            ERRORF(reporter, "Could not create surfaces for copy surface test.");
            continue;
        }

        SkCanvas* dstCanvas = dst->getCanvas();

        dstCanvas->clear(SK_ColorRED);
        dst->flush();

        SkPaint gammaPaint;
        gammaPaint.setBlendMode(SkBlendMode::kSrc);
        gammaPaint.setColorFilter(toSRGB ? SkColorFilters::LinearToSRGBGamma()
                                         : SkColorFilters::SRGBToLinearGamma());

        dstCanvas->drawBitmap(bm, 0, 0, &gammaPaint);
        dst->flush();

        sk_memset32(read.get(), 0, kW * kH);
        if (!dst->readPixels(ii, read.get(), kRowBytes, 0, 0)) {
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
                if (!check_gamma(s, r, toSRGB, error, &expected)) {
                    ERRORF(reporter, "Expected dst %d,%d to contain 0x%08x "
                           "from src 0x%08x and mode %s. Got %08x", x, y, expected, s,
                           toSRGB ? "ToSRGB" : "ToLinear", r);
                    abort = true;
                    break;
                }
            }
        }
    }
}
