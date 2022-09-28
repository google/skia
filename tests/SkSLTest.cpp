/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLProgramKind.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/SkSLVersion.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/Resources.h"
#include "tools/gpu/GrContextFactory.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

static constexpr int kWidth = 2;
static constexpr int kHeight = 2;

namespace SkSLTestFlags {
    /** CPU tests must pass on the CPU backend. */
    static constexpr int CPU     = 1 << 0;

    /** GPU tests must pass on all GPU backends. */
    static constexpr int GPU     = 1 << 1;

    /** GPU_ES3 tests must pass on ES3-compatible GPUs when "enforce ES2 restrictions" is off. */
    static constexpr int GPU_ES3 = 1 << 2;

    /** UsesNaN tests rely on NaN values, so they are only expected to pass on GPUs that generate
     *  them (which is not a requirement, even with ES3).
     */
    static constexpr int UsesNaN = 1 << 4;
}

static constexpr bool is_cpu(int flags) {
    return flags & SkSLTestFlags::CPU;
}

static constexpr bool is_gpu(int flags) {
    return flags & (SkSLTestFlags::GPU | SkSLTestFlags::GPU_ES3);
}

static constexpr bool is_strict_es2(int flags) {
    return !(flags & SkSLTestFlags::GPU_ES3);
}

template <typename T>
static void set_uniform(SkRuntimeShaderBuilder* builder, const char* name, const T& value) {
    SkRuntimeShaderBuilder::BuilderUniform uniform = builder->uniform(name);
    if (uniform.fVar) {
        uniform = value;
    }
}

template <typename T>
static void set_uniform_array(SkRuntimeShaderBuilder* builder, const char* name, SkSpan<T> values) {
    SkRuntimeShaderBuilder::BuilderUniform uniform = builder->uniform(name);
    if (uniform.fVar) {
        uniform.set(values.data(), values.size());
    }
}

static SkBitmap bitmap_from_shader(skiatest::Reporter* r,
                                   SkSurface* surface,
                                   sk_sp<SkRuntimeEffect> effect) {
    static constexpr float kArray[5] = {1, 2, 3, 4, 5};

    SkRuntimeShaderBuilder builder(effect);
    set_uniform(&builder, "colorBlack",       SkV4{0, 0, 0, 1});
    set_uniform(&builder, "colorRed",         SkV4{1, 0, 0, 1});
    set_uniform(&builder, "colorGreen",       SkV4{0, 1, 0, 1});
    set_uniform(&builder, "colorBlue",        SkV4{0, 0, 1, 1});
    set_uniform(&builder, "colorWhite",       SkV4{1, 1, 1, 1});
    set_uniform(&builder, "testInputs",       SkV4{-1.25, 0, 0.75, 2.25});
    set_uniform(&builder, "unknownInput",     1.0f);
    set_uniform(&builder, "testMatrix2x2",    std::array<float,4>{1, 2,
                                                                  3, 4});
    set_uniform(&builder, "testMatrix3x3",    std::array<float,9>{1, 2, 3,
                                                                  4, 5, 6,
                                                                  7, 8, 9});
    set_uniform(&builder, "testMatrix4x4",    std::array<float,16>{1,  2,  3,  4,
                                                                   5,  6,  7,  8,
                                                                   9,  10, 11, 12,
                                                                   13, 14, 15, 16});
    set_uniform_array(&builder, "testArray",  SkSpan(kArray));

    sk_sp<SkShader> shader = builder.makeShader();
    if (!shader) {
        return SkBitmap{};
    }

    surface->getCanvas()->clear(SK_ColorBLACK);

    SkPaint paintShader;
    paintShader.setShader(shader);
    surface->getCanvas()->drawRect(SkRect::MakeWH(kWidth, kHeight), paintShader);

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));
    REPORTER_ASSERT(r, surface->readPixels(bitmap, /*srcX=*/0, /*srcY=*/0));
    return bitmap;
}

static bool gpu_generates_nan(skiatest::Reporter* r, GrDirectContext* ctx) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    // The Metal shader compiler (which is also used under-the-hood for some GL/GLES contexts on
    // these platforms) enables fast-math by default. That prevents NaN-based tests from passing:
    // https://developer.apple.com/documentation/metal/mtlcompileoptions/1515914-fastmathenabled
    return false;
#else
    // If we don't have infinity support, we definitely won't generate NaNs
    if (!ctx->priv().caps()->shaderCaps()->fInfinitySupport) {
        return false;
    }

    auto effect = SkRuntimeEffect::MakeForShader(SkString(R"(
        #version 300
        uniform half4 colorGreen, colorRed;

        half4 main(float2 xy) {
            return isnan(colorGreen.r / colorGreen.b) ? colorGreen : colorRed;
        }
    )")).effect;
    REPORTER_ASSERT(r, effect);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    SkBitmap bitmap = bitmap_from_shader(r, surface.get(), effect);
    REPORTER_ASSERT(r, !bitmap.empty());

    SkColor color = bitmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SK_ColorGREEN || color == SK_ColorRED);
    return color == SK_ColorGREEN;
#endif
}

static SkString load_source(skiatest::Reporter* r,
                            const char* testFile,
                            const char* permutationSuffix) {
    SkString resourcePath = SkStringPrintf("sksl/%s", testFile);
    sk_sp<SkData> shaderData = GetResourceAsData(resourcePath.c_str());
    if (!shaderData) {
        ERRORF(r, "%s%s: Unable to load file", testFile, permutationSuffix);
        return SkString("");
    }
    return SkString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
}

static void test_one_permutation(skiatest::Reporter* r,
                                 SkSurface* surface,
                                 const char* testFile,
                                 const char* permutationSuffix,
                                 const SkRuntimeEffect::Options& options) {
    SkString shaderString = load_source(r, testFile, permutationSuffix);
    if (shaderString.isEmpty()) {
        return;
    }
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(shaderString, options);
    if (!result.effect) {
        ERRORF(r, "%s%s: %s", testFile, permutationSuffix, result.errorText.c_str());
        return;
    }

    SkBitmap bitmap = bitmap_from_shader(r, surface, result.effect);
    if (bitmap.empty()) {
        ERRORF(r, "%s%s: Unable to build shader", testFile, permutationSuffix);
        return;
    }

    bool success = true;
    SkColor color[kHeight][kWidth];
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            color[y][x] = bitmap.getColor(x, y);
            if (color[y][x] != SK_ColorGREEN) {
                success = false;
            }
        }
    }

    if (!success) {
        static_assert(kWidth  == 2);
        static_assert(kHeight == 2);
        ERRORF(r, "Expected%s: solid green. Actual:\n"
                  "RRGGBBAA RRGGBBAA\n"
                  "%02X%02X%02X%02X %02X%02X%02X%02X\n"
                  "%02X%02X%02X%02X %02X%02X%02X%02X",
                  permutationSuffix,
                  SkColorGetR(color[0][0]), SkColorGetG(color[0][0]),
                  SkColorGetB(color[0][0]), SkColorGetA(color[0][0]),

                  SkColorGetR(color[0][1]), SkColorGetG(color[0][1]),
                  SkColorGetB(color[0][1]), SkColorGetA(color[0][1]),

                  SkColorGetR(color[1][0]), SkColorGetG(color[1][0]),
                  SkColorGetB(color[1][0]), SkColorGetA(color[1][0]),

                  SkColorGetR(color[1][1]), SkColorGetG(color[1][1]),
                  SkColorGetB(color[1][1]), SkColorGetA(color[1][1]));
    }
}

static void test_permutations(skiatest::Reporter* r,
                              SkSurface* surface,
                              const char* testFile,
                              bool strictES2) {
    SkRuntimeEffect::Options options =
            strictES2 ? SkRuntimeEffect::Options{} : SkRuntimeEffectPriv::ES3Options();
    options.forceUnoptimized = false;
    test_one_permutation(r, surface, testFile, "", options);

    options.forceUnoptimized = true;
    test_one_permutation(r, surface, testFile, " (Unoptimized)", options);
}

static void test_cpu(skiatest::Reporter* r, const char* testFile, int flags) {
    SkASSERT(flags & SkSLTestFlags::CPU);

    // Create a raster-backed surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, const char* testFile, int flags) {
    // If this is an ES3-only test on a GPU which doesn't support SkSL ES3, return immediately.
    bool shouldRunGPU = (flags & SkSLTestFlags::GPU);
    bool shouldRunGPU_ES3 =
            (flags & SkSLTestFlags::GPU_ES3) &&
            (ctx->priv().caps()->shaderCaps()->supportedSkSLVerion() >= SkSL::Version::k300);
    if (!shouldRunGPU && !shouldRunGPU_ES3) {
        return;
    }

    // If this is a test that requires the GPU to generate NaN values, check for that first.
    if (flags & SkSLTestFlags::UsesNaN) {
        if (!gpu_generates_nan(r, ctx)) {
            return;
        }
    }

    // Create a GPU-backed surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    if (shouldRunGPU) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
    }
    if (shouldRunGPU_ES3) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/false);
    }
}

static void test_clone(skiatest::Reporter* r, const char* testFile, int flags) {
    SkString shaderString = load_source(r, testFile, "");
    if (shaderString.isEmpty()) {
        return;
    }
    SkSL::ProgramSettings settings;
    settings.fAllowVarDeclarationCloneForTesting = true;
    // TODO(skia:11209): Can we just put the correct #version in the source files that need this?
    settings.fMaxVersionAllowed = is_strict_es2(flags) ? SkSL::Version::k100 : SkSL::Version::k300;
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::ProgramKind::kRuntimeShader, shaderString.c_str(), settings);
    if (!program) {
        ERRORF(r, "%s", compiler.errorText().c_str());
        return;
    }
    // Starting DSL allows us to get access to the ThreadContext::Settings
    SkSL::dsl::Start(&compiler, SkSL::ProgramKind::kFragment, settings);
    for (const std::unique_ptr<SkSL::ProgramElement>& element : program->fOwnedElements) {
        std::string original = element->description();
        std::string cloned = element->clone()->description();
        REPORTER_ASSERT(r, original == cloned,
                "Mismatch after clone!\nOriginal: %s\nCloned: %s\n", original.c_str(),
                cloned.c_str());
    }
    SkSL::dsl::End();
}

#define SKSL_TEST(flags, ctsEnforcement, name, path)                                       \
    DEF_CONDITIONAL_TEST(SkSL##name##_CPU, r, is_cpu(flags)) { test_cpu(r, path, flags); } \
    DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(                                    \
            SkSL##name##_GPU, r, ctxInfo, is_gpu(flags), ctsEnforcement) {                 \
        test_gpu(r, ctxInfo.directContext(), path, flags);                                 \
    }                                                                                      \
    DEF_TEST(SkSL##name##_Clone, r) { test_clone(r, path, flags); }

/**
 * Test flags:
 * - CPU:     this test should pass on the CPU backend
 * - GPU:     this test should pass on the Ganesh GPU backends
 * - GPU_ES3: this test should pass on an ES3-compatible GPU when "enforce ES2 restrictions" is off
 *
 * CtsEnforcement:
 *   Android CTS (go/wtf/cts) enforces that devices must pass this test at the given API level.
 *   CTS and Android SkQP builds should only run tests on devices greater than the provided API
 *   level, but other test binaries (dm/fm) should run every test, regardless of this value.
 */

// clang-format off

using namespace SkSLTestFlags;

constexpr auto kApiLevel_T = CtsEnforcement::kApiLevel_T;
constexpr auto kNever = CtsEnforcement::kNever;
constexpr auto kNextRelease = CtsEnforcement::kNextRelease;

SKSL_TEST(CPU + GPU, kApiLevel_T, ArraySizeFolding,                "folding/ArraySizeFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, AssignmentOps,                   "folding/AssignmentOps.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, BoolFolding,                     "folding/BoolFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, CastFolding,                     "folding/CastFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntFoldingES2,                   "folding/IntFoldingES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntFoldingES3,                   "folding/IntFoldingES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, FloatFolding,                    "folding/FloatFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixFoldingES2,                "folding/MatrixFoldingES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      MatrixFoldingES3,                "folding/MatrixFoldingES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixNoOpFolding,               "folding/MatrixNoOpFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixScalarNoOpFolding,         "folding/MatrixScalarNoOpFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixVectorNoOpFolding,         "folding/MatrixVectorNoOpFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Negation,                        "folding/Negation.sksl")
// TODO(skia:13035): This test fails on Nvidia GPUs on OpenGL but passes Vulkan. Re-enable the test
// on Vulkan when granular GPU backend selection is supported.
SKSL_TEST(CPU, kApiLevel_T,       PreserveSideEffects,             "folding/PreserveSideEffects.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SelfAssignment,                  "folding/SelfAssignment.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ShortCircuitBoolFolding,         "folding/ShortCircuitBoolFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StructFieldFolding,              "folding/StructFieldFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StructFieldNoFolding,            "folding/StructFieldNoFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwitchCaseFolding,               "folding/SwitchCaseFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleFolding,                  "folding/SwizzleFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryFolding,                  "folding/TernaryFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, VectorScalarFolding,             "folding/VectorScalarFolding.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, VectorVectorFolding,             "folding/VectorVectorFolding.sksl")

SKSL_TEST(GPU_ES3,   kNever,      DoWhileBodyMustBeInlinedIntoAScope,               "inliner/DoWhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,   kNever,      DoWhileTestCannotBeInlined,                       "inliner/DoWhileTestCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ForBodyMustBeInlinedIntoAScope,                   "inliner/ForBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ForInitializerExpressionsCanBeInlined,            "inliner/ForInitializerExpressionsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ForWithoutReturnInsideCanBeInlined,               "inliner/ForWithoutReturnInsideCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ForWithReturnInsideCannotBeInlined,               "inliner/ForWithReturnInsideCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IfBodyMustBeInlinedIntoAScope,                    "inliner/IfBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IfElseBodyMustBeInlinedIntoAScope,                "inliner/IfElseBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IfElseChainWithReturnsCanBeInlined,               "inliner/IfElseChainWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IfTestCanBeInlined,                               "inliner/IfTestCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IfWithReturnsCanBeInlined,                        "inliner/IfWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineKeywordOverridesThreshold,                  "inliner/InlineKeywordOverridesThreshold.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlinerAvoidsVariableNameOverlap,                 "inliner/InlinerAvoidsVariableNameOverlap.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlinerElidesTempVarForReturnsInsideBlock,        "inliner/InlinerElidesTempVarForReturnsInsideBlock.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlinerUsesTempVarForMultipleReturns,             "inliner/InlinerUsesTempVarForMultipleReturns.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlinerUsesTempVarForReturnsInsideBlockWithVar,   "inliner/InlinerUsesTempVarForReturnsInsideBlockWithVar.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineThreshold,                                  "inliner/InlineThreshold.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineWithModifiedArgument,                       "inliner/InlineWithModifiedArgument.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineWithNestedBigCalls,                         "inliner/InlineWithNestedBigCalls.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineWithUnmodifiedArgument,                     "inliner/InlineWithUnmodifiedArgument.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InlineWithUnnecessaryBlocks,                      "inliner/InlineWithUnnecessaryBlocks.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, NoInline,                                         "inliner/NoInline.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ShortCircuitEvaluationsCannotInlineRightHandSide, "inliner/ShortCircuitEvaluationsCannotInlineRightHandSide.sksl")
SKSL_TEST(GPU_ES3,   kNever,      StaticSwitchInline,                               "inliner/StaticSwitch.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StructsCanBeInlinedSafely,                        "inliner/StructsCanBeInlinedSafely.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleCanBeInlinedDirectly,                      "inliner/SwizzleCanBeInlinedDirectly.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryResultsCannotBeInlined,                    "inliner/TernaryResultsCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryTestCanBeInlined,                          "inliner/TernaryTestCanBeInlined.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TrivialArgumentsInlineDirectly,                   "inliner/TrivialArgumentsInlineDirectly.sksl")
SKSL_TEST(GPU_ES3,   kNever,      TrivialArgumentsInlineDirectlyES3,                "inliner/TrivialArgumentsInlineDirectlyES3.sksl")
SKSL_TEST(GPU_ES3,   kNever,      WhileBodyMustBeInlinedIntoAScope,                 "inliner/WhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,   kNever,      WhileTestCannotBeInlined,                         "inliner/WhileTestCannotBeInlined.sksl")

SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicAbsFloat,               "intrinsics/AbsFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicAbsInt,                 "intrinsics/AbsInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicCeil,                   "intrinsics/Ceil.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicDeterminant,            "intrinsics/Determinant.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicDFdx,                   "intrinsics/DFdx.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicDFdy,                   "intrinsics/DFdy.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicFloatBitsToInt,         "intrinsics/FloatBitsToInt.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicFloatBitsToUint,        "intrinsics/FloatBitsToUint.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicFwidth,                 "intrinsics/Fwidth.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicIntBitsToFloat,         "intrinsics/IntBitsToFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicIsInf,                  "intrinsics/IsInf.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicClampInt,               "intrinsics/ClampInt.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicClampUInt,              "intrinsics/ClampUInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicClampFloat,             "intrinsics/ClampFloat.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicMatrixCompMultES2,      "intrinsics/MatrixCompMultES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicMatrixCompMultES3,      "intrinsics/MatrixCompMultES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicMaxFloat,               "intrinsics/MaxFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicMaxInt,                 "intrinsics/MaxInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicMinFloat,               "intrinsics/MinFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicMinInt,                 "intrinsics/MinInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicMixFloat,               "intrinsics/MixFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicModf,                   "intrinsics/Modf.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicOuterProduct,           "intrinsics/OuterProduct.sksl")
// Fails on Mac OpenGL + Radeon 5300M (skia:12434)
// SKSL_TEST(GPU_ES3,   kNever,      IntrinsicPackUnorm2x16,          "intrinsics/PackUnorm2x16.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicRound,                  "intrinsics/Round.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicRoundEven,              "intrinsics/RoundEven.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicSignFloat,              "intrinsics/SignFloat.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicSignInt,                "intrinsics/SignInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, IntrinsicStep,                   "intrinsics/Step.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicTrunc,                  "intrinsics/Trunc.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicTranspose,              "intrinsics/Transpose.sksl")
SKSL_TEST(GPU_ES3,   kNever,      IntrinsicUintBitsToFloat,        "intrinsics/UintBitsToFloat.sksl")

SKSL_TEST(GPU_ES3,   kNever,      ArrayNarrowingConversions,       "runtime/ArrayNarrowingConversions.rts")
SKSL_TEST(CPU + GPU, kApiLevel_T, LoopFloat,                       "runtime/LoopFloat.rts")
SKSL_TEST(CPU + GPU, kApiLevel_T, LoopInt,                         "runtime/LoopInt.rts")
SKSL_TEST(CPU + GPU, kApiLevel_T, QualifierOrder,                  "runtime/QualifierOrder.rts")
SKSL_TEST(CPU + GPU, kApiLevel_T, PrecisionQualifiers,             "runtime/PrecisionQualifiers.rts")

SKSL_TEST(GPU_ES3 + UsesNaN, kNever, RecursiveComparison_Arrays,     "runtime/RecursiveComparison_Arrays.rts")
SKSL_TEST(GPU_ES3 + UsesNaN, kNever, RecursiveComparison_Structs,    "runtime/RecursiveComparison_Structs.rts")
SKSL_TEST(GPU_ES3 + UsesNaN, kNever, RecursiveComparison_Types,      "runtime/RecursiveComparison_Types.rts")
SKSL_TEST(GPU_ES3 + UsesNaN, kNever, RecursiveComparison_Vectors,    "runtime/RecursiveComparison_Vectors.rts")

SKSL_TEST(GPU_ES3,   kNever,      ArrayCast,                       "shared/ArrayCast.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ArrayComparison,                 "shared/ArrayComparison.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ArrayConstructors,               "shared/ArrayConstructors.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ArrayFollowedByScalar,           "shared/ArrayFollowedByScalar.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ArrayTypes,                      "shared/ArrayTypes.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Assignment,                      "shared/Assignment.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, CastsRoundTowardZero,            "shared/CastsRoundTowardZero.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, CommaMixedTypes,                 "shared/CommaMixedTypes.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, CommaSideEffects,                "shared/CommaSideEffects.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ConstantIf,                      "shared/ConstantIf.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ConstArray,                      "shared/ConstArray.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ConstVariableComparison,         "shared/ConstVariableComparison.sksl")
SKSL_TEST(GPU_ES3,   kNever,      DeadLoopVariable,                "shared/DeadLoopVariable.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, DeadIfStatement,                 "shared/DeadIfStatement.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, DeadReturn,                      "shared/DeadReturn.sksl")
// TODO(skia:12012): some Radeons crash when compiling this code; disable them.
// SKSL_TEST(GPU_ES3,kNever,      SkSLDeadReturnES3,               "shared/DeadReturnES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, DeadStripFunctions,              "shared/DeadStripFunctions.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, DependentInitializers,           "shared/DependentInitializers.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, DoubleNegation,                  "shared/DoubleNegation.sksl")
SKSL_TEST(GPU_ES3,   kNever,      DoWhileControlFlow,              "shared/DoWhileControlFlow.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, EmptyBlocksES2,                  "shared/EmptyBlocksES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      EmptyBlocksES3,                  "shared/EmptyBlocksES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ForLoopControlFlow,              "shared/ForLoopControlFlow.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, FunctionAnonymousParameters,     "shared/FunctionAnonymousParameters.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, FunctionArgTypeMatch,            "shared/FunctionArgTypeMatch.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, FunctionReturnTypeMatch,         "shared/FunctionReturnTypeMatch.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Functions,                       "shared/Functions.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, FunctionPrototype,               "shared/FunctionPrototype.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, GeometricIntrinsics,             "shared/GeometricIntrinsics.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, HelloWorld,                      "shared/HelloWorld.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Hex,                             "shared/Hex.sksl")
SKSL_TEST(GPU_ES3,   kNever,      HexUnsigned,                     "shared/HexUnsigned.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InoutParameters,                 "shared/InoutParameters.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, InoutParamsAreDistinct,          "shared/InoutParamsAreDistinct.sksl")
SKSL_TEST(GPU_ES3,   kApiLevel_T, IntegerDivisionES3,              "shared/IntegerDivisionES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Matrices,                        "shared/Matrices.sksl")
SKSL_TEST(GPU_ES3,   kNever,      MatricesNonsquare,               "shared/MatricesNonsquare.sksl")
// TODO(skia:12443) These tests actually don't work on MANY devices. The GLSL SkQP suite
// does a terrible job of enforcing this rule. We still test the ES2 variant on CPU.
SKSL_TEST(CPU,       kNever,      MatrixConstructorsES2,           "shared/MatrixConstructorsES2.sksl")
// SKSL_TEST(GPU_ES3, kNever,     MatrixConstructorsES3,           "shared/MatrixConstructorsES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixEquality,                  "shared/MatrixEquality.sksl")
SKSL_TEST(GPU_ES3,   kNextRelease,MatrixOpEqualsES3,               "shared/MatrixOpEqualsES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixScalarMath,                "shared/MatrixScalarMath.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MatrixToVectorCast,              "shared/MatrixToVectorCast.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, MultipleAssignments,             "shared/MultipleAssignments.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, NumberCasts,                     "shared/NumberCasts.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, OperatorsES2,                    "shared/OperatorsES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      OperatorsES3,                    "shared/OperatorsES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Ossfuzz36852,                    "shared/Ossfuzz36852.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, OutParams,                       "shared/OutParams.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, OutParamsAreDistinct,            "shared/OutParamsAreDistinct.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, OutParamsAreDistinctFromGlobal,  "shared/OutParamsAreDistinctFromGlobal.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, OutParamsTricky,                 "shared/OutParamsTricky.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ResizeMatrix,                    "shared/ResizeMatrix.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ResizeMatrixNonsquare,           "shared/ResizeMatrixNonsquare.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ReturnsValueOnEveryPathES2,      "shared/ReturnsValueOnEveryPathES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ReturnsValueOnEveryPathES3,      "shared/ReturnsValueOnEveryPathES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ScalarConversionConstructorsES2, "shared/ScalarConversionConstructorsES2.sksl")
SKSL_TEST(GPU_ES3,   kNever,      ScalarConversionConstructorsES3, "shared/ScalarConversionConstructorsES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, ScopedSymbol,                    "shared/ScopedSymbol.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StackingVectorCasts,             "shared/StackingVectorCasts.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StaticIf,                        "shared/StaticIf.sksl")
SKSL_TEST(GPU_ES3,   kNever,      StaticSwitch,                    "shared/StaticSwitch.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StructArrayFollowedByScalar,     "shared/StructArrayFollowedByScalar.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, StructsInFunctions,              "shared/StructsInFunctions.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, Switch,                          "shared/Switch.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwitchDefaultOnly,               "shared/SwitchDefaultOnly.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwitchWithFallthrough,           "shared/SwitchWithFallthrough.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwitchWithLoops,                 "shared/SwitchWithLoops.sksl")
SKSL_TEST(GPU_ES3,   kNever,      SwitchWithLoopsES3,              "shared/SwitchWithLoopsES3.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleBoolConstants,            "shared/SwizzleBoolConstants.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleByConstantIndex,          "shared/SwizzleByConstantIndex.sksl")
SKSL_TEST(GPU_ES3,   kNever,      SwizzleByIndex,                  "shared/SwizzleByIndex.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleConstants,                "shared/SwizzleConstants.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleLTRB,                     "shared/SwizzleLTRB.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleOpt,                      "shared/SwizzleOpt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleScalar,                   "shared/SwizzleScalar.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleScalarBool,               "shared/SwizzleScalarBool.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, SwizzleScalarInt,                "shared/SwizzleScalarInt.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryAsLValueEntirelyFoldable, "shared/TernaryAsLValueEntirelyFoldable.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryAsLValueFoldableTest,     "shared/TernaryAsLValueFoldableTest.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, TernaryExpression,               "shared/TernaryExpression.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, UnaryPositiveNegative,           "shared/UnaryPositiveNegative.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, UniformArray,                    "shared/UniformArray.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, UniformMatrixResize,             "shared/UniformMatrixResize.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, UnusedVariables,                 "shared/UnusedVariables.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, VectorConstructors,              "shared/VectorConstructors.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, VectorToMatrixCast,              "shared/VectorToMatrixCast.sksl")
SKSL_TEST(CPU + GPU, kApiLevel_T, VectorScalarMath,                "shared/VectorScalarMath.sksl")
SKSL_TEST(GPU_ES3,   kNever,      WhileLoopControlFlow,            "shared/WhileLoopControlFlow.sksl")
