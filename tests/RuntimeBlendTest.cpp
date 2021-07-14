/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLDefines.h"  // for kDefaultInlineThreshold
#include "include/utils/SkRandom.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/RuntimeBlendUtils.h"
#include "tools/ToolUtils.h"

static bool nearly_equal(const SkColor& x, const SkColor& y) {
    const int kTolerance = 1;
    return abs((int)SkColorGetA(x) - (int)SkColorGetA(y)) <= kTolerance &&
           abs((int)SkColorGetR(x) - (int)SkColorGetR(y)) <= kTolerance &&
           abs((int)SkColorGetG(x) - (int)SkColorGetG(y)) <= kTolerance &&
           abs((int)SkColorGetB(x) - (int)SkColorGetB(y)) <= kTolerance;
}

static void test_blend(skiatest::Reporter* r, SkSurface* surface, SkBlendMode mode) {
    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));

    for (int alpha : {0x80, 0xFF}) {
        std::vector<SkColor> colors;
        for (bool useRuntimeBlend : {false, true}) {
            // Draw a solid red pixel.
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setBlendMode(SkBlendMode::kSrc);
            surface->getCanvas()->drawRect(SkRect::MakeWH(1, 1), paint);

            // Draw a blue pixel on top of it, using the passed-in blend mode.
            paint.setColor(SkColorSetARGB(alpha, 0x00, 0x00, 0xFF));
            if (useRuntimeBlend) {
                paint.setBlender(GetRuntimeBlendForBlendMode(mode));
            } else {
                paint.setBlendMode(mode);
            }
            surface->getCanvas()->drawRect(SkRect::MakeWH(1, 1), paint);

            // Read back the red/blue blended pixel.
            REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(),
                                                   bitmap.rowBytes(), /*srcX=*/0, /*srcY=*/0));
            colors.push_back(bitmap.getColor(/*x=*/0, /*y=*/0));
        }

        REPORTER_ASSERT(r, nearly_equal(colors[0], colors[1]),
                        "Expected: %s blends match. Actual: Built-in A=%02X R=%02X G=%02X B=%02X, "
                        "Runtime A=%02X R=%02X G=%02X B=%02X",
                        (alpha = 0xFF) ? "solid" : "transparent",
                        SkColorGetA(colors[0]),
                        SkColorGetR(colors[0]),
                        SkColorGetG(colors[0]),
                        SkColorGetB(colors[0]),
                        SkColorGetA(colors[1]),
                        SkColorGetR(colors[1]),
                        SkColorGetG(colors[1]),
                        SkColorGetB(colors[1]));
    }
}

static void test_cpu(skiatest::Reporter* r, SkBlendMode mode) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(/*width=*/1, /*height=*/1);
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    test_blend(r, surface.get(), mode);
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, SkBlendMode mode) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(/*width=*/1, /*height=*/1);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    test_blend(r, surface.get(), mode);
}

#define BLEND_TEST_CPU(mode)                     \
    DEF_TEST(SkRuntimeBlender_##mode##_CPU, r) { \
        test_cpu(r, SkBlendMode::k##mode);       \
    }
#define BLEND_TEST_GPU(mode)                                                        \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRuntimeBlender_##mode##_GPU, r, ctxInfo) { \
        test_gpu(r, ctxInfo.directContext(), SkBlendMode::k##mode);                 \
    }
#define BLEND_TEST(mode) BLEND_TEST_CPU(mode) BLEND_TEST_GPU(mode)

BLEND_TEST(Clear)
BLEND_TEST(Src)
BLEND_TEST(Dst)
BLEND_TEST(SrcOver)
BLEND_TEST(DstOver)
BLEND_TEST(SrcIn)
BLEND_TEST(DstIn)
BLEND_TEST(SrcOut)
BLEND_TEST(DstOut)
BLEND_TEST(SrcATop)
BLEND_TEST(DstATop)
BLEND_TEST(Xor)
BLEND_TEST(Plus)
BLEND_TEST(Modulate)
BLEND_TEST(Screen)
BLEND_TEST(Overlay)
BLEND_TEST(Darken)
BLEND_TEST(Lighten)
BLEND_TEST(ColorDodge)
BLEND_TEST(ColorBurn)
BLEND_TEST(HardLight)
BLEND_TEST(SoftLight)
BLEND_TEST(Difference)
BLEND_TEST(Exclusion)
BLEND_TEST(Multiply)
BLEND_TEST(Hue)
BLEND_TEST(Saturation)
BLEND_TEST(Color)
BLEND_TEST(Luminosity)
