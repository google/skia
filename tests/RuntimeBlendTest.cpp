/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h" // IWYU pragma: keep
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/RuntimeBlendUtils.h"

#include <cmath>
#include <initializer_list>
#include <vector>

struct GrContextOptions;

static bool nearly_equal(const SkColor& x, const SkColor& y) {
    const int kTolerance = 1;
    return abs((int)SkColorGetA(x) - (int)SkColorGetA(y)) <= kTolerance &&
           abs((int)SkColorGetR(x) - (int)SkColorGetR(y)) <= kTolerance &&
           abs((int)SkColorGetG(x) - (int)SkColorGetG(y)) <= kTolerance &&
           abs((int)SkColorGetB(x) - (int)SkColorGetB(y)) <= kTolerance;
}

static void test_blend(skiatest::Reporter* r, SkSurface* surface) {
    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));

    for (int m = 0; m < kSkBlendModeCount; ++m) {
        SkBlendMode mode = (SkBlendMode)m;
        for (int alpha : {0x80, 0xFF}) {
            for (bool useShader : {false, true}) {
                std::vector<SkColor> colors;
                for (bool useRuntimeBlend : {false, true}) {
                    // Draw a solid red pixel.
                    SkPaint paint;
                    paint.setColor(SK_ColorRED);
                    paint.setBlendMode(SkBlendMode::kSrc);
                    surface->getCanvas()->drawRect(SkRect::MakeWH(1, 1), paint);

                    // Draw a blue pixel on top of it, using the passed-in blend mode.
                    if (useShader) {
                        // Install a different color in the paint, to ensure we're using the shader
                        paint.setColor(SK_ColorGREEN);
                        paint.setShader(SkShaders::Color(SkColorSetARGB(alpha, 0x00, 0x00, 0xFF)));
                    } else {
                        paint.setColor(SkColorSetARGB(alpha, 0x00, 0x00, 0xFF));
                    }
                    if (useRuntimeBlend) {
                        paint.setBlender(GetRuntimeBlendForBlendMode(mode));
                    } else {
                        paint.setBlendMode(mode);
                    }
                    surface->getCanvas()->drawRect(SkRect::MakeWH(1, 1), paint);

                    // Read back the red/blue blended pixel.
                    REPORTER_ASSERT(r,
                                    surface->readPixels(bitmap.info(),
                                                        bitmap.getPixels(),
                                                        bitmap.rowBytes(),
                                                        /*srcX=*/0,
                                                        /*srcY=*/0));
                    colors.push_back(bitmap.getColor(/*x=*/0, /*y=*/0));
                }

                REPORTER_ASSERT(r,
                                nearly_equal(colors[0], colors[1]),
                                "Expected: %s %s %s blend matches. Actual: Built-in "
                                "A=%02X R=%02X G=%02X B=%02X, Runtime A=%02X R=%02X G=%02X B=%02X",
                                SkBlendMode_Name(mode),
                                (alpha == 0xFF) ? "solid" : "transparent",
                                useShader ? "shader" : "paint",
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
    }
}

DEF_TEST(SkRuntimeBlender_CPU, r) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(/*width=*/1, /*height=*/1);
    sk_sp<SkSurface> surface(SkSurfaces::Raster(info));

    test_blend(r, surface.get());
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeBlender_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(/*width=*/1, /*height=*/1);
    sk_sp<SkSurface> surface(
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info));
    test_blend(r, surface.get());
}
