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
#include "include/sksl/DSLCore.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDehydrator.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static constexpr int kWidth = 2;
static constexpr int kHeight = 2;

namespace SkSLTestFlags {
    /** CPU tests must pass on the CPU backend. */
    static constexpr int CPU     = 1 << 0;

    /** CPU_ES3 tests must pass on the CPU backend when "enforce ES2 restrictions" is off. */
    static constexpr int CPU_ES3 = 1 << 1;

    /** GPU tests must pass on all GPU backends. */
    static constexpr int GPU     = 1 << 2;

    /** GPU_ES3 tests must pass on ES3-compatible GPUs when "enforce ES2 restrictions" is off. */
    static constexpr int GPU_ES3 = 1 << 3;

    /** SkQP tests will be run in Android/Fuchsia conformance tests with no driver workarounds. */
    static constexpr int SkQP    = 1 << 4;
}

static constexpr bool is_cpu(int flags) {
    return flags & (SkSLTestFlags::CPU | SkSLTestFlags::CPU_ES3);
}

static constexpr bool is_gpu(int flags) {
    return flags & (SkSLTestFlags::GPU | SkSLTestFlags::GPU_ES3);
}

static constexpr bool is_strict_es2(int flags) {
    return !(flags & (SkSLTestFlags::CPU_ES3 | SkSLTestFlags::GPU_ES3));
}

static bool should_run_in_skqp(int flags) {
    if (CurrentTestHarnessIsSkQP()) {
        // Official SkQP builds should only run tests marked with the SkQP flag.
        return flags & (SkSLTestFlags::SkQP);
    } else {
        // Other test binaries (dm/fm) should run every test, regardless of the SkQP flag.
        return true;
    }
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

    static constexpr float kArray[5] = {1, 2, 3, 4, 5};

    SkRuntimeShaderBuilder builder(result.effect);
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
    set_uniform_array(&builder, "testArray",  SkMakeSpan(kArray));

    sk_sp<SkShader> shader = builder.makeShader();
    if (!shader) {
        ERRORF(r, "%s%s: Unable to build shader", testFile, permutationSuffix);
        return;
    }

    surface->getCanvas()->clear(SK_ColorBLACK);

    SkPaint paintShader;
    paintShader.setShader(shader);
    surface->getCanvas()->drawRect(SkRect::MakeWH(kWidth, kHeight), paintShader);

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));
    REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                           /*srcX=*/0, /*srcY=*/0));

    bool success = true;
    SkColor color[kHeight][kWidth];
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            color[y][x] = bitmap.getColor(x, y);
            if (color[y][x] != SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00)) {
                success = false;
            }
        }
    }

    if (!success) {
        static_assert(kWidth  == 2);
        static_assert(kHeight == 2);
        ERRORF(r, "Expected: solid green. Actual:\n"
                  "RRGGBBAA RRGGBBAA\n"
                  "%02X%02X%02X%02X %02X%02X%02X%02X\n"
                  "%02X%02X%02X%02X %02X%02X%02X%02X",
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
    options.forceNoInline = false;
    test_one_permutation(r, surface, testFile, "", options);

    options.forceNoInline = true;
    test_one_permutation(r, surface, testFile, " (NoInline)", options);
}

static void test_cpu(skiatest::Reporter* r, const char* testFile, int flags) {
    bool shouldRunCPU = (flags & SkSLTestFlags::CPU);
    bool shouldRunCPU_ES3 = (flags & SkSLTestFlags::CPU_ES3);
    SkASSERT(shouldRunCPU || shouldRunCPU_ES3);

    // Create a raster-backed surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));

    if (shouldRunCPU) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
    }
    if (shouldRunCPU_ES3) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/false);
    }
}

static void test_gpu(skiatest::Reporter* r, GrDirectContext* ctx, const char* testFile, int flags) {
    // If this is an ES3-only test on a GPU which doesn't support SkSL ES3, return immediately.
    bool shouldRunGPU = (flags & SkSLTestFlags::GPU);
    bool shouldRunGPU_ES3 = (flags & SkSLTestFlags::GPU_ES3) &&
                            ctx->priv().caps()->shaderCaps()->supportsSkSLES3();
    if (!shouldRunGPU && !shouldRunGPU_ES3) {
        return;
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
    std::unique_ptr<SkSL::ShaderCaps> caps = SkSL::ShaderCapsFactory::Standalone();
    SkSL::Program::Settings settings;
    settings.fAllowVarDeclarationCloneForTesting = true;
    settings.fEnforceES2Restrictions = is_strict_es2(flags);
    SkSL::Compiler compiler(caps.get());
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

static void test_rehydrate(skiatest::Reporter* r, const char* testFile, int flags) {
    SkString shaderString = load_source(r, testFile, "");
    if (shaderString.isEmpty()) {
        return;
    }
    std::unique_ptr<SkSL::ShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    SkSL::Compiler compiler(caps.get());
    SkSL::Program::Settings settings;
    settings.fEnforceES2Restrictions = is_strict_es2(flags);
    // Inlining causes problems because it can create expressions like bool(1) that can't be
    // directly instantiated. After a dehydrate/recycle pass, that expression simply becomes "true"
    // due to optimization - which is fine, but would cause us to fail an equality comparison. We
    // disable inlining to avoid this issue.
    settings.fInlineThreshold = 0;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::ProgramKind::kRuntimeShader, shaderString.c_str(), settings);
    if (!program) {
        ERRORF(r, "%s", compiler.errorText().c_str());
        return;
    }
    SkSL::Dehydrator dehydrator;
    dehydrator.write(*program);
    SkSL::StringStream stream;
    dehydrator.finish(stream);

    SkSL::Rehydrator rehydrator(compiler, (const uint8_t*) stream.str().data(),
            stream.str().length());
    std::unique_ptr<SkSL::Program> rehydrated = rehydrator.program();
    REPORTER_ASSERT(r, rehydrated->description() == program->description(),
            "Mismatch between original and dehydrated/rehydrated:\n-- Original:\n%s\n"
            "-- Rehydrated:\n%s", program->description().c_str(),
            rehydrated->description().c_str());
}

#define SKSL_TEST(flags, name, path)                                                        \
    DEF_CONDITIONAL_TEST(SkSL##name##_CPU, r, is_cpu(flags) && should_run_in_skqp(flags)) { \
        test_cpu(r, path, flags);                                                           \
    }                                                                                       \
    DEF_CONDITIONAL_GPUTEST_FOR_RENDERING_CONTEXTS(                                         \
            SkSL##name##_GPU, r, ctxInfo, is_gpu(flags) && should_run_in_skqp(flags)) {     \
        test_gpu(r, ctxInfo.directContext(), path, flags);                                  \
    }                                                                                       \
    DEF_TEST(SkSL##name##_Clone, r) { test_clone(r, path, flags); }                         \
    DEF_TEST(SkSL##name##_Rehydrate, r) { test_rehydrate(r, path, flags); }

/**
 * Test flags:
 * - CPU:     this test should pass on the CPU backend
 * - CPU_ES3: this test should pass on the CPU backend when "enforce ES2 restrictions" is off
 * - GPU:     this test should pass on the GPU backends
 * - GPU_ES3: this test should pass on an ES3-compatible GPU when "enforce ES2 restrictions" is off
 * - SkQP:    Android CTS (go/wtf/cts) enforces that devices must pass this test
 */

// clang-format off

using namespace SkSLTestFlags;

SKSL_TEST(CPU + GPU + SkQP, ArraySizeFolding,                "folding/ArraySizeFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, AssignmentOps,                   "folding/AssignmentOps.sksl")
SKSL_TEST(CPU + GPU + SkQP, BoolFolding,                     "folding/BoolFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, CastFolding,                     "folding/CastFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntFoldingES2,                   "folding/IntFoldingES2.sksl")
SKSL_TEST(GPU_ES3,          IntFoldingES3,                   "folding/IntFoldingES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, FloatFolding,                    "folding/FloatFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, MatrixFoldingES2,                "folding/MatrixFoldingES2.sksl")
SKSL_TEST(GPU_ES3,          MatrixFoldingES3,                "folding/MatrixFoldingES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, Negation,                        "folding/Negation.sksl")
SKSL_TEST(CPU + GPU + SkQP, PreserveSideEffects,             "folding/PreserveSideEffects.sksl")
SKSL_TEST(CPU + GPU + SkQP, SelfAssignment,                  "folding/SelfAssignment.sksl")
SKSL_TEST(CPU + GPU + SkQP, ShortCircuitBoolFolding,         "folding/ShortCircuitBoolFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwitchCaseFolding,               "folding/SwitchCaseFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleFolding,                  "folding/SwizzleFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, VectorScalarFolding,             "folding/VectorScalarFolding.sksl")
SKSL_TEST(CPU + GPU + SkQP, VectorVectorFolding,             "folding/VectorVectorFolding.sksl")

SKSL_TEST(GPU_ES3,          DoWhileBodyMustBeInlinedIntoAScope,               "inliner/DoWhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,          DoWhileTestCannotBeInlined,                       "inliner/DoWhileTestCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, ForBodyMustBeInlinedIntoAScope,                   "inliner/ForBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,          ForInitializerExpressionsCanBeInlined,            "inliner/ForInitializerExpressionsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, ForWithoutReturnInsideCanBeInlined,               "inliner/ForWithoutReturnInsideCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, ForWithReturnInsideCannotBeInlined,               "inliner/ForWithReturnInsideCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, IfBodyMustBeInlinedIntoAScope,                    "inliner/IfBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU + GPU + SkQP, IfElseBodyMustBeInlinedIntoAScope,                "inliner/IfElseBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU + GPU + SkQP, IfElseChainWithReturnsCanBeInlined,               "inliner/IfElseChainWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, IfTestCanBeInlined,                               "inliner/IfTestCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, IfWithReturnsCanBeInlined,                        "inliner/IfWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineKeywordOverridesThreshold,                  "inliner/InlineKeywordOverridesThreshold.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlinerAvoidsVariableNameOverlap,                 "inliner/InlinerAvoidsVariableNameOverlap.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlinerElidesTempVarForReturnsInsideBlock,        "inliner/InlinerElidesTempVarForReturnsInsideBlock.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlinerUsesTempVarForMultipleReturns,             "inliner/InlinerUsesTempVarForMultipleReturns.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlinerUsesTempVarForReturnsInsideBlockWithVar,   "inliner/InlinerUsesTempVarForReturnsInsideBlockWithVar.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineThreshold,                                  "inliner/InlineThreshold.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineWithModifiedArgument,                       "inliner/InlineWithModifiedArgument.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineWithNestedBigCalls,                         "inliner/InlineWithNestedBigCalls.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineWithUnmodifiedArgument,                     "inliner/InlineWithUnmodifiedArgument.sksl")
SKSL_TEST(CPU + GPU + SkQP, InlineWithUnnecessaryBlocks,                      "inliner/InlineWithUnnecessaryBlocks.sksl")
SKSL_TEST(CPU + GPU + SkQP, NoInline,                                         "inliner/NoInline.sksl")
SKSL_TEST(CPU + GPU + SkQP, ShortCircuitEvaluationsCannotInlineRightHandSide, "inliner/ShortCircuitEvaluationsCannotInlineRightHandSide.sksl")
SKSL_TEST(GPU_ES3,          StaticSwitchInline,                               "inliner/StaticSwitch.sksl")
SKSL_TEST(CPU + GPU + SkQP, StructsCanBeInlinedSafely,                        "inliner/StructsCanBeInlinedSafely.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleCanBeInlinedDirectly,                      "inliner/SwizzleCanBeInlinedDirectly.sksl")
SKSL_TEST(CPU + GPU + SkQP, TernaryResultsCannotBeInlined,                    "inliner/TernaryResultsCannotBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, TernaryTestCanBeInlined,                          "inliner/TernaryTestCanBeInlined.sksl")
SKSL_TEST(CPU + GPU + SkQP, TrivialArgumentsInlineDirectly,                   "inliner/TrivialArgumentsInlineDirectly.sksl")
SKSL_TEST(GPU_ES3,          WhileBodyMustBeInlinedIntoAScope,                 "inliner/WhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(GPU_ES3,          WhileTestCannotBeInlined,                         "inliner/WhileTestCannotBeInlined.sksl")

SKSL_TEST(CPU + GPU + SkQP, IntrinsicAbsFloat,               "intrinsics/AbsFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicAbsInt,                 "intrinsics/AbsInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicCeil,                   "intrinsics/Ceil.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicDeterminant,            "intrinsics/Determinant.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicDFdx,                   "intrinsics/DFdx.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicDFdy,                   "intrinsics/DFdy.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicFloatBitsToInt,         "intrinsics/FloatBitsToInt.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicFloatBitsToUint,        "intrinsics/FloatBitsToUint.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicFwidth,                 "intrinsics/Fwidth.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicIntBitsToFloat,         "intrinsics/IntBitsToFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicIsInf,                  "intrinsics/IsInf.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicClampInt,               "intrinsics/ClampInt.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicClampUInt,              "intrinsics/ClampUInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicClampFloat,             "intrinsics/ClampFloat.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicMatrixCompMultES2,      "intrinsics/MatrixCompMultES2.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicMatrixCompMultES3,      "intrinsics/MatrixCompMultES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicMaxFloat,               "intrinsics/MaxFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicMaxInt,                 "intrinsics/MaxInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicMinFloat,               "intrinsics/MinFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicMinInt,                 "intrinsics/MinInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicMixFloat,               "intrinsics/MixFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicModf,                   "intrinsics/Modf.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicOuterProduct,           "intrinsics/OuterProduct.sksl")
// Fails on Mac OpenGL + Radeon 5300M (skia:12434)
// SKSL_TEST(GPU_ES3,       IntrinsicPackUnorm2x16,          "intrinsics/PackUnorm2x16.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicRound,                  "intrinsics/Round.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicRoundEven,              "intrinsics/RoundEven.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicSignFloat,              "intrinsics/SignFloat.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicSignInt,                "intrinsics/SignInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, IntrinsicStep,                   "intrinsics/Step.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicTrunc,                  "intrinsics/Trunc.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicTranspose,              "intrinsics/Transpose.sksl")
SKSL_TEST(GPU_ES3,          IntrinsicUintBitsToFloat,        "intrinsics/UintBitsToFloat.sksl")

SKSL_TEST(GPU_ES3,          ArrayNarrowingConversions,       "runtime/ArrayNarrowingConversions.rts")
SKSL_TEST(CPU + GPU + SkQP, LoopFloat,                       "runtime/LoopFloat.rts")
SKSL_TEST(CPU + GPU + SkQP, LoopInt,                         "runtime/LoopInt.rts")
SKSL_TEST(CPU + GPU + SkQP, QualifierOrder,                  "runtime/QualifierOrder.rts")
SKSL_TEST(CPU + GPU + SkQP, PrecisionQualifiers,             "runtime/PrecisionQualifiers.rts")

// These tests specifically rely the behavior of NaN values, but some older GPUs do not reliably
// implement full IEEE support (skia:12977). They also rely on equality operators on array types
// which are not available in GLSL ES 1.00. Therefore these tests are restricted to run on CPU and
// with "strict ES2 mode" disabled.
SKSL_TEST(CPU_ES3,          RecursiveComparison_Arrays,      "runtime/RecursiveComparison_Arrays.rts")
SKSL_TEST(CPU_ES3,          RecursiveComparison_Structs,     "runtime/RecursiveComparison_Structs.rts")
SKSL_TEST(CPU_ES3,          RecursiveComparison_Types,       "runtime/RecursiveComparison_Types.rts")
SKSL_TEST(CPU_ES3,          RecursiveComparison_Vectors,     "runtime/RecursiveComparison_Vectors.rts")

SKSL_TEST(GPU_ES3,          ArrayCast,                       "shared/ArrayCast.sksl")
SKSL_TEST(GPU_ES3,          ArrayComparison,                 "shared/ArrayComparison.sksl")
SKSL_TEST(GPU_ES3,          ArrayConstructors,               "shared/ArrayConstructors.sksl")
SKSL_TEST(GPU_ES3,          ArrayFollowedByScalar,           "shared/ArrayFollowedByScalar.sksl")
SKSL_TEST(CPU + GPU + SkQP, ArrayTypes,                      "shared/ArrayTypes.sksl")
SKSL_TEST(CPU + GPU + SkQP, Assignment,                      "shared/Assignment.sksl")
SKSL_TEST(CPU + GPU + SkQP, CastsRoundTowardZero,            "shared/CastsRoundTowardZero.sksl")
SKSL_TEST(CPU + GPU + SkQP, CommaMixedTypes,                 "shared/CommaMixedTypes.sksl")
SKSL_TEST(CPU + GPU + SkQP, CommaSideEffects,                "shared/CommaSideEffects.sksl")
SKSL_TEST(CPU + GPU + SkQP, ConstantIf,                      "shared/ConstantIf.sksl")
SKSL_TEST(GPU_ES3,          ConstArray,                      "shared/ConstArray.sksl")
SKSL_TEST(CPU + GPU + SkQP, ConstVariableComparison,         "shared/ConstVariableComparison.sksl")
SKSL_TEST(GPU_ES3,          DeadLoopVariable,                "shared/DeadLoopVariable.sksl")
SKSL_TEST(CPU + GPU + SkQP, DeadIfStatement,                 "shared/DeadIfStatement.sksl")
SKSL_TEST(CPU + GPU + SkQP, DeadReturn,                      "shared/DeadReturn.sksl")
// TODO(skia:12012): some Radeons crash when compiling this code; disable them.
// SKSL_TEST(GPU_ES3,       SkSLDeadReturnES3,               "shared/DeadReturnES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, DeadStripFunctions,              "shared/DeadStripFunctions.sksl")
SKSL_TEST(CPU + GPU + SkQP, DependentInitializers,           "shared/DependentInitializers.sksl")
SKSL_TEST(GPU_ES3,          DoWhileControlFlow,              "shared/DoWhileControlFlow.sksl")
SKSL_TEST(CPU + GPU + SkQP, EmptyBlocksES2,                  "shared/EmptyBlocksES2.sksl")
SKSL_TEST(GPU_ES3,          EmptyBlocksES3,                  "shared/EmptyBlocksES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, ForLoopControlFlow,              "shared/ForLoopControlFlow.sksl")
SKSL_TEST(CPU + GPU + SkQP, FunctionAnonymousParameters,     "shared/FunctionAnonymousParameters.sksl")
SKSL_TEST(CPU + GPU + SkQP, FunctionArgTypeMatch,            "shared/FunctionArgTypeMatch.sksl")
SKSL_TEST(CPU + GPU + SkQP, FunctionReturnTypeMatch,         "shared/FunctionReturnTypeMatch.sksl")
SKSL_TEST(CPU + GPU + SkQP, Functions,                       "shared/Functions.sksl")
SKSL_TEST(CPU + GPU + SkQP, FunctionPrototype,               "shared/FunctionPrototype.sksl")
SKSL_TEST(CPU + GPU + SkQP, GeometricIntrinsics,             "shared/GeometricIntrinsics.sksl")
SKSL_TEST(CPU + GPU + SkQP, HelloWorld,                      "shared/HelloWorld.sksl")
SKSL_TEST(CPU + GPU + SkQP, Hex,                             "shared/Hex.sksl")
SKSL_TEST(GPU_ES3,          HexUnsigned,                     "shared/HexUnsigned.sksl")
SKSL_TEST(CPU + GPU + SkQP, InoutParameters,                 "shared/InoutParameters.sksl")
SKSL_TEST(CPU + GPU + SkQP, Matrices,                        "shared/Matrices.sksl")
SKSL_TEST(GPU_ES3,          MatricesNonsquare,               "shared/MatricesNonsquare.sksl")
// TODO(skia:12443) These tests actually don't work on MANY devices. The GLSL SkQP suite
// does a terrible job of enforcing this rule. We still test them on CPU.
SKSL_TEST(CPU,              MatrixConstructorsES2,           "shared/MatrixConstructorsES2.sksl")
SKSL_TEST(CPU_ES3,          MatrixConstructorsES3,           "shared/MatrixConstructorsES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, MatrixEquality,                  "shared/MatrixEquality.sksl")
SKSL_TEST(CPU + GPU + SkQP, MatrixScalarMath,                "shared/MatrixScalarMath.sksl")
SKSL_TEST(CPU + GPU + SkQP, MatrixToVectorCast,              "shared/MatrixToVectorCast.sksl")
SKSL_TEST(CPU + GPU + SkQP, MultipleAssignments,             "shared/MultipleAssignments.sksl")
SKSL_TEST(CPU + GPU + SkQP, NumberCasts,                     "shared/NumberCasts.sksl")
SKSL_TEST(CPU + GPU + SkQP, OperatorsES2,                    "shared/OperatorsES2.sksl")
SKSL_TEST(GPU_ES3,          OperatorsES3,                    "shared/OperatorsES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, Ossfuzz36852,                    "shared/Ossfuzz36852.sksl")
SKSL_TEST(CPU + GPU + SkQP, OutParams,                       "shared/OutParams.sksl")
SKSL_TEST(CPU + GPU + SkQP, OutParamsAreDistinct,            "shared/OutParamsAreDistinct.sksl")
SKSL_TEST(CPU + GPU + SkQP, OutParamsTricky,                 "shared/OutParamsTricky.sksl")
SKSL_TEST(CPU + GPU + SkQP, ResizeMatrix,                    "shared/ResizeMatrix.sksl")
SKSL_TEST(GPU_ES3,          ResizeMatrixNonsquare,           "shared/ResizeMatrixNonsquare.sksl")
SKSL_TEST(CPU + GPU + SkQP, ReturnsValueOnEveryPathES2,      "shared/ReturnsValueOnEveryPathES2.sksl")
SKSL_TEST(GPU_ES3,          ReturnsValueOnEveryPathES3,      "shared/ReturnsValueOnEveryPathES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, ScalarConversionConstructorsES2, "shared/ScalarConversionConstructorsES2.sksl")
SKSL_TEST(GPU_ES3,          ScalarConversionConstructorsES3, "shared/ScalarConversionConstructorsES3.sksl")
SKSL_TEST(CPU + GPU + SkQP, ScopedSymbol,                    "shared/ScopedSymbol.sksl")
SKSL_TEST(CPU + GPU + SkQP, StackingVectorCasts,             "shared/StackingVectorCasts.sksl")
SKSL_TEST(CPU + GPU + SkQP, StaticIf,                        "shared/StaticIf.sksl")
SKSL_TEST(GPU_ES3,          StaticSwitch,                    "shared/StaticSwitch.sksl")
SKSL_TEST(CPU + GPU + SkQP, StructArrayFollowedByScalar,     "shared/StructArrayFollowedByScalar.sksl")
SKSL_TEST(CPU + GPU + SkQP, StructsInFunctions,              "shared/StructsInFunctions.sksl")
SKSL_TEST(CPU + GPU + SkQP, Switch,                          "shared/Switch.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwitchDefaultOnly,               "shared/SwitchDefaultOnly.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwitchWithFallthrough,           "shared/SwitchWithFallthrough.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwitchWithLoops,                 "shared/SwitchWithLoops.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleBoolConstants,            "shared/SwizzleBoolConstants.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleByConstantIndex,          "shared/SwizzleByConstantIndex.sksl")
SKSL_TEST(GPU_ES3,          SwizzleByIndex,                  "shared/SwizzleByIndex.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleConstants,                "shared/SwizzleConstants.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleLTRB,                     "shared/SwizzleLTRB.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleOpt,                      "shared/SwizzleOpt.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleScalar,                   "shared/SwizzleScalar.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleScalarBool,               "shared/SwizzleScalarBool.sksl")
SKSL_TEST(CPU + GPU + SkQP, SwizzleScalarInt,                "shared/SwizzleScalarInt.sksl")
SKSL_TEST(CPU + GPU + SkQP, TernaryAsLValueEntirelyFoldable, "shared/TernaryAsLValueEntirelyFoldable.sksl")
SKSL_TEST(CPU + GPU + SkQP, TernaryAsLValueFoldableTest,     "shared/TernaryAsLValueFoldableTest.sksl")
SKSL_TEST(CPU + GPU + SkQP, TernaryExpression,               "shared/TernaryExpression.sksl")
SKSL_TEST(CPU + GPU + SkQP, UnaryPositiveNegative,           "shared/UnaryPositiveNegative.sksl")
SKSL_TEST(CPU + GPU + SkQP, UniformArray,                    "shared/UniformArray.sksl")
SKSL_TEST(CPU + GPU + SkQP, UnusedVariables,                 "shared/UnusedVariables.sksl")
SKSL_TEST(CPU + GPU + SkQP, VectorConstructors,              "shared/VectorConstructors.sksl")
SKSL_TEST(CPU + GPU + SkQP, VectorToMatrixCast,              "shared/VectorToMatrixCast.sksl")
SKSL_TEST(CPU + GPU + SkQP, VectorScalarMath,                "shared/VectorScalarMath.sksl")
SKSL_TEST(GPU_ES3,          WhileLoopControlFlow,            "shared/WhileLoopControlFlow.sksl")
