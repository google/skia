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

static const SkRect kRect = SkRect::MakeWH(1, 1);

template <typename T>
static void set_uniform(SkRuntimeShaderBuilder* builder, const char* name, const T& value) {
    SkRuntimeShaderBuilder::BuilderUniform uniform = builder->uniform(name);
    if (uniform.fVar) {
        uniform = value;
    }
}

static void test(skiatest::Reporter* r, SkSurface* surface, const char* testFile) {
    SkString resourcePath = SkStringPrintf("sksl/%s", testFile);
    sk_sp<SkData> shaderData = GetResourceAsData(resourcePath.c_str());
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", testFile);
        return;
    }

    SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
    auto [effect, error] = SkRuntimeEffect::Make(shaderString);
    if (!effect) {
        ERRORF(r, "%s: %s", testFile, error.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(effect);
    set_uniform(&builder, "colorBlack",   SkV4{0, 0, 0, 1});
    set_uniform(&builder, "colorRed",     SkV4{1, 0, 0, 1});
    set_uniform(&builder, "colorGreen",   SkV4{0, 1, 0, 1});
    set_uniform(&builder, "colorBlue",    SkV4{0, 0, 1, 1});
    set_uniform(&builder, "colorWhite",   SkV4{1, 1, 1, 1});
    set_uniform(&builder, "unknownInput", 1.0f);

    sk_sp<SkShader> shader = builder.makeShader(/*localMatrix=*/nullptr, /*isOpaque=*/true);
    if (!shader) {
        ERRORF(r, "%s: Unable to build shader", testFile);
        return;
    }

    SkPaint paintShader;
    paintShader.setShader(shader);
    surface->getCanvas()->drawRect(kRect, paintShader);

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));
    REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                           /*srcX=*/0, /*srcY=*/0));

    SkColor color = bitmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00),
                    "Expected: solid green. Actual: A=%02X R=%02X G=%02X B=%02X.",
                    SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
}

static void test_cpu(skiatest::Reporter* r, const char* testFile) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kRect.width(), kRect.height());
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    test(r, surface.get(), testFile);
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, const char* testFile) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kRect.width(), kRect.height());
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    test(r, surface.get(), testFile);
}

#define SKSL_TEST(name, path)                                       \
    DEF_TEST(name ## _CPU, r) {                                     \
        test_cpu(r, path);                                          \
    }                                                               \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name ## _GPU, r, ctxInfo) {  \
        test_gpu(r, ctxInfo.directContext(), path);                 \
    }

SKSL_TEST(SkSLBoolFolding,             "folding/BoolFolding.sksl")
SKSL_TEST(SkSLIntFoldingES2,           "folding/IntFoldingES2.sksl")
SKSL_TEST(SkSLFloatFolding,            "folding/FloatFolding.sksl")
SKSL_TEST(SkSLMatrixFoldingES2,        "folding/MatrixFoldingES2.sksl")
SKSL_TEST(SkSLShortCircuitBoolFolding, "folding/ShortCircuitBoolFolding.sksl")
SKSL_TEST(SkSLVectorScalarFolding,     "folding/VectorScalarFolding.sksl")
SKSL_TEST(SkSLVectorVectorFolding,     "folding/VectorVectorFolding.sksl")

SKSL_TEST(SkSLForLoopControlFlow,      "shared/ForLoopControlFlow.sksl")

/*
TODO(skia:11209): enable these tests when Runtime Effects have support for ES3

SKSL_TEST(SkSLIntFoldingES3,           "folding/IntFoldingES3.sksl")
SKSL_TEST(SkSLMatrixFoldingES3,        "folding/MatrixFoldingES3.sksl")

SKSL_TEST(SkSLDoWhileControlFlow,      "shared/DoWhileControlFlow.sksl")
SKSL_TEST(SkSLWhileLoopControlFlow,    "shared/WhileLoopControlFlow.sksl")
*/
