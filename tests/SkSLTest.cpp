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

static void test_one_permutation(skiatest::Reporter* r,
                                 SkSurface* surface,
                                 const char* testFile,
                                 const char* permutationSuffix,
                                 const SkRuntimeEffect::Options& options) {
    SkString resourcePath = SkStringPrintf("sksl/%s", testFile);
    sk_sp<SkData> shaderData = GetResourceAsData(resourcePath.c_str());
    if (!shaderData) {
        ERRORF(r, "%s%s: Unable to load file", testFile, permutationSuffix);
        return;
    }

    SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
    SkRuntimeEffect::Result result = SkRuntimeEffect::Make(shaderString, options);
    if (!result.effect) {
        ERRORF(r, "%s%s: %s", testFile, permutationSuffix, result.errorText.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(result.effect);
    set_uniform(&builder, "colorBlack",       SkV4{0, 0, 0, 1});
    set_uniform(&builder, "colorRed",         SkV4{1, 0, 0, 1});
    set_uniform(&builder, "colorGreen",       SkV4{0, 1, 0, 1});
    set_uniform(&builder, "colorBlue",        SkV4{0, 0, 1, 1});
    set_uniform(&builder, "colorWhite",       SkV4{1, 1, 1, 1});
    set_uniform(&builder, "testInputs",       SkV4{-1.25, 0, 0.75, 2.25});
    set_uniform(&builder, "testMatrix2x2",    std::array<float,4>{1, 2,
                                                                  3, 4});
    set_uniform(&builder, "testMatrix3x3",    std::array<float,9>{1, 2, 3,
                                                                  4, 5, 6,
                                                                  7, 8, 9});
    set_uniform(&builder, "unknownInput",     1.0f);
    set_uniform(&builder, "testMatrix2x2",    std::array<float,4>{1, 2,
                                                                  3, 4});
    set_uniform(&builder, "testMatrix3x3",    std::array<float,9>{1, 2, 3,
                                                                  4, 5, 6,
                                                                  7, 8, 9});

    sk_sp<SkShader> shader = builder.makeShader(/*localMatrix=*/nullptr, /*isOpaque=*/true);
    if (!shader) {
        ERRORF(r, "%s%s: Unable to build shader", testFile, permutationSuffix);
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

static void test_permutations(skiatest::Reporter* r, SkSurface* surface, const char* testFile) {
    SkRuntimeEffect::Options options;
    options.inlineThreshold = 0;
    test_one_permutation(r, surface, testFile, " (NoInline)", options);

    options.inlineThreshold = SkSL::kDefaultInlineThreshold;
    test_one_permutation(r, surface, testFile, "", options);
}

static void test_cpu(skiatest::Reporter* r, const char* testFile) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kRect.width(), kRect.height());
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    test_permutations(r, surface.get(), testFile);
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, const char* testFile) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kRect.width(), kRect.height());
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    test_permutations(r, surface.get(), testFile);
}

#define SKSL_TEST_CPU(name, path)                                   \
    DEF_TEST(name ## _CPU, r) {                                     \
        test_cpu(r, path);                                          \
    }
#define SKSL_TEST_GPU(name, path)                                   \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name ## _GPU, r, ctxInfo) {  \
        test_gpu(r, ctxInfo.directContext(), path);                 \
    }
#define SKSL_TEST(name, path) SKSL_TEST_CPU(name, path) SKSL_TEST_GPU(name, path)

SKSL_TEST(SkSLAssignmentOps,                   "folding/AssignmentOps.sksl")
SKSL_TEST(SkSLBoolFolding,                     "folding/BoolFolding.sksl")
SKSL_TEST(SkSLIntFoldingES2,                   "folding/IntFoldingES2.sksl")
SKSL_TEST(SkSLFloatFolding,                    "folding/FloatFolding.sksl")
SKSL_TEST(SkSLMatrixFoldingES2,                "folding/MatrixFoldingES2.sksl")
SKSL_TEST(SkSLSelfAssignment,                  "folding/SelfAssignment.sksl")
SKSL_TEST(SkSLShortCircuitBoolFolding,         "folding/ShortCircuitBoolFolding.sksl")
SKSL_TEST(SkSLVectorScalarFolding,             "folding/VectorScalarFolding.sksl")
SKSL_TEST(SkSLVectorVectorFolding,             "folding/VectorVectorFolding.sksl")

// TODO(skia:11052): SPIR-V does not yet honor `out` param semantics correctly
SKSL_TEST_CPU(SkSLInlinerHonorsGLSLOutParamSemantics,
              "inliner/InlinerHonorsGLSLOutParamSemantics.sksl")

SKSL_TEST(SkSLIntrinsicAbsFloat,               "intrinsics/AbsFloat.sksl")
SKSL_TEST(SkSLIntrinsicCeil,                   "intrinsics/Ceil.sksl")
SKSL_TEST(SkSLIntrinsicClampFloat,             "intrinsics/ClampFloat.sksl")
SKSL_TEST(SkSLIntrinsicMaxFloat,               "intrinsics/MaxFloat.sksl")
SKSL_TEST(SkSLIntrinsicMinFloat,               "intrinsics/MinFloat.sksl")
SKSL_TEST(SkSLIntrinsicMixFloat,               "intrinsics/MixFloat.sksl")
SKSL_TEST(SkSLIntrinsicSignFloat,              "intrinsics/SignFloat.sksl")

SKSL_TEST(SkSLArrayTypes,                      "shared/ArrayTypes.sksl")
SKSL_TEST(SkSLAssignment,                      "shared/Assignment.sksl")
SKSL_TEST(SkSLCastsRoundTowardZero,            "shared/CastsRoundTowardZero.sksl")
SKSL_TEST(SkSLCommaMixedTypes,                 "shared/CommaMixedTypes.sksl")
// This test causes the Adreno 330 driver to crash, and does not pass on Quadro P400 in wasm.
// The CPU test confirms that we can get it right, even if not all drivers do.
SKSL_TEST_CPU(SkSLCommaSideEffects,            "shared/CommaSideEffects.sksl")
SKSL_TEST(SkSLConstantIf,                      "shared/ConstantIf.sksl")
SKSL_TEST(SkSLConstVariableComparison,         "shared/ConstVariableComparison.sksl")
SKSL_TEST(SkSLDeadIfStatement,                 "shared/DeadIfStatement.sksl")
SKSL_TEST(SkSLDeadStripFunctions,              "shared/DeadStripFunctions.sksl")
SKSL_TEST(SkSLDependentInitializers,           "shared/DependentInitializers.sksl")
SKSL_TEST(SkSLEmptyBlocksES2,                  "shared/EmptyBlocksES2.sksl")
SKSL_TEST(SkSLForLoopControlFlow,              "shared/ForLoopControlFlow.sksl")
SKSL_TEST(SkSLFunctionArgTypeMatch,            "shared/FunctionArgTypeMatch.sksl")
SKSL_TEST(SkSLFunctionReturnTypeMatch,         "shared/FunctionReturnTypeMatch.sksl")
SKSL_TEST(SkSLFunctions,                       "shared/Functions.sksl")
SKSL_TEST(SkSLGeometricIntrinsics,             "shared/GeometricIntrinsics.sksl")
SKSL_TEST(SkSLHelloWorld,                      "shared/HelloWorld.sksl")
SKSL_TEST(SkSLHex,                             "shared/Hex.sksl")
SKSL_TEST(SkSLMatrices,                        "shared/Matrices.sksl")
SKSL_TEST(SkSLMatrixEquality,                  "shared/MatrixEquality.sksl")
SKSL_TEST(SkSLMultipleAssignments,             "shared/MultipleAssignments.sksl")
SKSL_TEST(SkSLNegatedVectorLiteral,            "shared/NegatedVectorLiteral.sksl")
SKSL_TEST(SkSLNumberCasts,                     "shared/NumberCasts.sksl")
SKSL_TEST(SkSLOperatorsES2,                    "shared/OperatorsES2.sksl")
SKSL_TEST(SkSLOutParams,                       "shared/OutParams.sksl")
SKSL_TEST(SkSLOutParamsNoInline,               "shared/OutParamsNoInline.sksl")
SKSL_TEST(SkSLOutParamsTricky,                 "shared/OutParamsTricky.sksl")
SKSL_TEST(SkSLResizeMatrix,                    "shared/ResizeMatrix.sksl")
SKSL_TEST(SkSLReturnsValueOnEveryPathES2,      "shared/ReturnsValueOnEveryPathES2.sksl")
SKSL_TEST(SkSLScalarConversionConstructorsES2, "shared/ScalarConversionConstructorsES2.sksl")
SKSL_TEST(SkSLStackingVectorCasts,             "shared/StackingVectorCasts.sksl")
SKSL_TEST(SkSLStaticIf,                        "shared/StaticIf.sksl")
SKSL_TEST(SkSLStructsInFunctions,              "shared/StructsInFunctions.sksl")
SKSL_TEST(SkSLSwizzleBoolConstants,            "shared/SwizzleBoolConstants.sksl")
SKSL_TEST(SkSLSwizzleByConstantIndex,          "shared/SwizzleByConstantIndex.sksl")
SKSL_TEST(SkSLSwizzleConstants,                "shared/SwizzleConstants.sksl")
SKSL_TEST(SkSLSwizzleLTRB,                     "shared/SwizzleLTRB.sksl")
SKSL_TEST(SkSLSwizzleOpt,                      "shared/SwizzleOpt.sksl")
SKSL_TEST(SkSLSwizzleScalar,                   "shared/SwizzleScalar.sksl")
SKSL_TEST(SkSLTernaryAsLValueEntirelyFoldable, "shared/TernaryAsLValueEntirelyFoldable.sksl")
SKSL_TEST(SkSLTernaryAsLValueFoldableTest,     "shared/TernaryAsLValueFoldableTest.sksl")
SKSL_TEST(SkSLTernaryExpression,               "shared/TernaryExpression.sksl")
SKSL_TEST(SkSLUnaryPositiveNegative,           "shared/UnaryPositiveNegative.sksl")
SKSL_TEST(SkSLUnusedVariables,                 "shared/UnusedVariables.sksl")
SKSL_TEST(SkSLVectorConstructors,              "shared/VectorConstructors.sksl")

/*
// Incompatible with Runtime Effects because calling a function before its definition is disallowed.
// (This was done to prevent recursion, as required by ES2.)
SKSL_TEST(SkSLFunctionPrototype,               "shared/FunctionPrototype.sksl")
*/

/*
TODO(skia:11209): enable these tests when Runtime Effects have support for ES3

SKSL_TEST(SkSLIntFoldingES3,                   "folding/IntFoldingES3.sksl")
SKSL_TEST(SkSLMatrixFoldingES3,                "folding/MatrixFoldingES3.sksl")

SKSL_TEST(SkSLIntrinsicAbsInt,                 "intrinsics/AbsInt.sksl")
SKSL_TEST(SkSLIntrinsicClampInt,               "intrinsics/ClampInt.sksl")
SKSL_TEST(SkSLIntrinsicMaxInt,                 "intrinsics/MaxInt.sksl")
SKSL_TEST(SkSLIntrinsicMinInt,                 "intrinsics/MinInt.sksl")
SKSL_TEST(SkSLIntrinsicMixBool,                "intrinsics/MixBool.sksl")
SKSL_TEST(SkSLIntrinsicSignInt,                "intrinsics/SignInt.sksl")

SKSL_TEST(SkSLArrayConstructors,               "shared/ArrayConstructors.sksl")
SKSL_TEST(SkSLDeadLoopVariable,                "shared/DeadLoopVariable.sksl")
SKSL_TEST(SkSLDoWhileControlFlow,              "shared/DoWhileControlFlow.sksl")
SKSL_TEST(SkSLEmptyBlocksES3,                  "shared/EmptyBlocksES3.sksl")
SKSL_TEST(SkSLHexUnsigned,                     "shared/HexUnsigned.sksl")
SKSL_TEST(SkSLMatricesNonsquare,               "shared/MatricesNonsquare.sksl")
SKSL_TEST(SkSLOperatorsES3,                    "shared/OperatorsES3.sksl")
SKSL_TEST(SkSLResizeMatrixNonsquare,           "shared/ResizeMatrixNonsquare.sksl")
SKSL_TEST(SkSLReturnsValueOnEveryPathES3,      "shared/ReturnsValueOnEveryPathES3.sksl")
SKSL_TEST(SkSLScalarConversionConstructorsES3, "shared/ScalarConversionConstructorsES3.sksl")
SKSL_TEST(SkSLSwizzleByIndex,                  "shared/SwizzleByIndex.sksl")
SKSL_TEST(SkSLWhileLoopControlFlow,            "shared/WhileLoopControlFlow.sksl")
*/
