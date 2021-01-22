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
#include "include/utils/SkRandom.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

sk_sp<SkShader> make_shader(skiatest::Reporter* r, const char* testFile) {
    SkString resourcePath = SkStringPrintf("sksl/%s", testFile);
    sk_sp<SkData> shaderData = GetResourceAsData(resourcePath.c_str());
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", testFile);
        return nullptr;
    }

    SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
    auto [effect, error] = SkRuntimeEffect::Make(shaderString);
    if (!effect) {
        ERRORF(r, "%s: %s", testFile, error.c_str());
        return nullptr;
    }

    SkRuntimeShaderBuilder builder(effect);
    sk_sp<SkShader> shader = builder.makeShader(/*localMatrix=*/nullptr, /*isOpaque=*/true);
    if (!shader) {
        ERRORF(r, "%s: Unable to build shader", testFile);
        return nullptr;
    }

    return shader;
}

static void test_cpu(skiatest::Reporter* r, const char* testFile) {
    sk_sp<SkShader> shader = make_shader(r, testFile);
    if (!shader) {
        return;
    }

    static const SkRect kRect = SkRect::MakeWH(1, 1);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(kRect.width(), kRect.height());

    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);

    SkPaint paintShader;
    paintShader.setShader(shader);
    canvas.drawRect(kRect, paintShader);

    SkColor color = bitmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00),
                    "Expected: solid green. Actual: A=%02X R=%02X G=%02X B=%02X.",
                    SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, const char* testFile) {
    sk_sp<SkShader> shader = make_shader(r, testFile);
    if (!shader) {
        return;
    }

    static const SkRect kRect = SkRect::MakeWH(1, 1);
    SkImageInfo info = SkImageInfo::Make(kRect.width(), kRect.height(),
                                         kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paintShader;
    paintShader.setShader(shader);
    canvas->drawRect(kRect, paintShader);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                           /*srcX=*/0, /*srcY=*/0));

    SkColor color = bitmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00),
                    "Expected: solid green. Actual: A=%02X R=%02X G=%02X B=%02X.",
                    SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
}

#define SKSL_TEST(name, path)                                       \
    DEF_TEST(name ## CPU, r) {                                      \
        test_cpu(r, path);                                          \
    }                                                               \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name ## GPU, r, ctxInfo) {   \
        test_gpu(r, ctxInfo.directContext(), path);                 \
    }

SKSL_TEST(SkSLBoolFolding,             "folding/BoolFolding.sksl")
SKSL_TEST(SkSLFloatFolding,            "folding/FloatFolding.sksl")
SKSL_TEST(SkSLIntFoldingES2,           "folding/IntFoldingES2.sksl")
SKSL_TEST(SkSLMatrixFoldingES2,        "folding/MatrixFoldingES2.sksl")
SKSL_TEST(SkSLShortCircuitBoolFolding, "folding/ShortCircuitBoolFolding.sksl")
SKSL_TEST(SkSLVectorScalarFolding,     "folding/VectorScalarFolding.sksl")
SKSL_TEST(SkSLVectorVectorFolding,     "folding/VectorVectorFolding.sksl")

/*
TODO(skia:11209): enable these tests when Runtime Effects have support for ES3

DEF_TEST(SkSLIntFoldingES3, r)           { test(r, "folding/IntFoldingES3.sksl"); }
DEF_TEST(SkSLMatrixFoldingES3, r)        { test(r, "folding/MatrixFoldingES3.sksl"); }
*/
