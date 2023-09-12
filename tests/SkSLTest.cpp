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
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLVersion.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#if defined(SK_DAWN)
#include "src/gpu/graphite/dawn/DawnCaps.h"
#endif
#endif

using namespace skia_private;

namespace SkSL { class Context; }
struct GrContextOptions;

static constexpr int kWidth = 2;
static constexpr int kHeight = 2;

enum class SkSLTestFlag : int {
    /** `CPU` tests must pass when painted to a CPU-backed surface via SkRuntimeEffect. */
    CPU     = 1 << 0,

    /**
     * `ES3` tests must pass when executed directly on the CPU via the SkRasterPipeline backend.
     * They aren't compatible with SkRuntimeEffect, since they use non-ES2 features.
     */
    ES3     = 1 << 1,

    /** `GPU` tests must pass when painted to a GPU-backed surface via SkRuntimeEffect. */
    GPU     = 1 << 2,

    /** `GPU_ES3` tests must pass on ES3-compatible GPUs when "enforce ES2 restrictions" is off. */
    GPU_ES3 = 1 << 3,

    /**
     * `UsesNaN` tests rely on NaN values, so they are only expected to pass on GPUs that generate
     * them (which is not a requirement, even with ES3).
     */
    UsesNaN = 1 << 4,
};

using SkSLTestFlags = SkEnumBitMask<SkSLTestFlag>;

static constexpr bool is_cpu(SkSLTestFlags flags) {
    return SkToBool(flags & SkSLTestFlag::CPU);
}

static constexpr bool is_gpu(SkSLTestFlags flags) {
    return (flags & SkSLTestFlag::GPU) || (flags & SkSLTestFlag::GPU_ES3);
}

static constexpr bool is_strict_es2(SkSLTestFlags flags) {
    return !(flags & SkSLTestFlag::GPU_ES3) && !(flags & SkSLTestFlag::ES3);
}

struct UniformData {
    std::string_view    name;
    SkSpan<const float> span;
};

static constexpr float kUniformColorBlack[]    = {0.0f, 0.0f, 0.0f, 1.0f};
static constexpr float kUniformColorRed  []    = {1.0f, 0.0f, 0.0f, 1.0f};
static constexpr float kUniformColorGreen[]    = {0.0f, 1.0f, 0.0f, 1.0f};
static constexpr float kUniformColorBlue []    = {0.0f, 0.0f, 1.0f, 1.0f};
static constexpr float kUniformColorWhite[]    = {1.0f, 1.0f, 1.0f, 1.0f};
static constexpr float kUniformTestInputs[]    = {-1.25f, 0.0f, 0.75f, 2.25f};
static constexpr float kUniformUnknownInput[]  = {1.0f};
static constexpr float kUniformTestMatrix2x2[] = {1.0f, 2.0f,
                                                  3.0f, 4.0f};
static constexpr float kUniformTestMatrix3x3[] = {1.0f, 2.0f, 3.0f,
                                                  4.0f, 5.0f, 6.0f,
                                                  7.0f, 8.0f, 9.0f};
static constexpr float kUniformTestMatrix4x4[] = {1.0f,  2.0f,  3.0f,  4.0f,
                                                  5.0f,  6.0f,  7.0f,  8.0f,
                                                  9.0f,  10.0f, 11.0f, 12.0f,
                                                  13.0f, 14.0f, 15.0f, 16.0f};
static constexpr float kUniformTestArray[] = {1, 2, 3, 4, 5};
static constexpr float kUniformTestArrayNegative[] = {-1, -2, -3, -4, -5};

static constexpr UniformData kUniformData[] = {
        {"colorBlack", kUniformColorBlack},
        {"colorRed", kUniformColorRed},
        {"colorGreen", kUniformColorGreen},
        {"colorBlue", kUniformColorBlue},
        {"colorWhite", kUniformColorWhite},
        {"testInputs", kUniformTestInputs},
        {"unknownInput", kUniformUnknownInput},
        {"testMatrix2x2", kUniformTestMatrix2x2},
        {"testMatrix3x3", kUniformTestMatrix3x3},
        {"testMatrix4x4", kUniformTestMatrix4x4},
        {"testArray", kUniformTestArray},
        {"testArrayNegative", kUniformTestArrayNegative},
};

static SkBitmap bitmap_from_shader(skiatest::Reporter* r,
                                   SkSurface* surface,
                                   sk_sp<SkRuntimeEffect> effect) {

    SkRuntimeShaderBuilder builder(effect);
    for (const UniformData& data : kUniformData) {
        SkRuntimeShaderBuilder::BuilderUniform uniform = builder.uniform(data.name);
        if (uniform.fVar) {
            uniform.set(data.span.data(), data.span.size());
        }
    }

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
    sk_sp<SkSurface> surface(SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info));

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
    SkRuntimeEffect::Options options = strictES2 ? SkRuntimeEffect::Options{}
                                                 : SkRuntimeEffectPriv::ES3Options();
    options.forceUnoptimized = false;
    test_one_permutation(r, surface, testFile, "", options);

    options.forceUnoptimized = true;
    test_one_permutation(r, surface, testFile, " (Unoptimized)", options);
}

static void test_cpu(skiatest::Reporter* r, const char* testFile, SkSLTestFlags flags) {
    SkASSERT(flags & SkSLTestFlag::CPU);

    // Create a raster-backed surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurfaces::Raster(info));

    test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
}

#if defined(SK_GANESH)
static void test_ganesh(skiatest::Reporter* r,
                        GrDirectContext* ctx,
                        const char* testFile,
                        SkSLTestFlags flags) {
    // If this is an ES3-only test on a GPU which doesn't support SkSL ES3, return immediately.
    bool shouldRunGPU = SkToBool(flags & SkSLTestFlag::GPU);
    bool shouldRunGPU_ES3 =
            (flags & SkSLTestFlag::GPU_ES3) &&
            (ctx->priv().caps()->shaderCaps()->supportedSkSLVerion() >= SkSL::Version::k300);
    if (!shouldRunGPU && !shouldRunGPU_ES3) {
        return;
    }

    // If this is a test that requires the GPU to generate NaN values, check for that first.
    if (flags & SkSLTestFlag::UsesNaN) {
        if (!gpu_generates_nan(r, ctx)) {
            return;
        }
    }

    // Create a GPU-backed Ganesh surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info));

    if (shouldRunGPU) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
    }
    if (shouldRunGPU_ES3) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/false);
    }
}
#endif

#if defined(SK_GRAPHITE)
static void test_graphite(skiatest::Reporter* r,
                          skgpu::graphite::Context* ctx,
                          const char* testFile,
                          SkSLTestFlags flags) {
    // If this is an ES3-only test on a GPU which doesn't support SkSL ES3, return immediately.
    bool shouldRunGPU = SkToBool(flags & SkSLTestFlag::GPU);
    bool shouldRunGPU_ES3 =
            (flags & SkSLTestFlag::GPU_ES3) &&
            (ctx->priv().caps()->shaderCaps()->supportedSkSLVerion() >= SkSL::Version::k300);
    if (!shouldRunGPU && !shouldRunGPU_ES3) {
        return;
    }

#if defined(SK_DAWN)
    if (ctx->backend() == skgpu::BackendApi::kDawn) {
        // If this is a test that requires the GPU to generate NaN values, we don't run it in Dawn.
        // (WGSL/Dawn does not support infinity or NaN even if the GPU natively does.)
        if (flags & SkSLTestFlag::UsesNaN) {
            return;
        }
    }
#endif

#if defined(SK_BUILD_FOR_UNIX) && !defined(SK_DEBUG)
    // TODO(b/297239696): re-enable these tests once Graphite works properly here
    return;
#else
    // Create a GPU-backed Graphite surface.
    std::unique_ptr<skgpu::graphite::Recorder> recorder = ctx->makeRecorder();

    const SkImageInfo info = SkImageInfo::Make({kWidth, kHeight},
                                                kRGBA_8888_SkColorType,
                                                kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);

    if (shouldRunGPU) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/true);
    }
    if (shouldRunGPU_ES3) {
        test_permutations(r, surface.get(), testFile, /*strictES2=*/false);
    }
#endif
}
#endif

static void test_clone(skiatest::Reporter* r, const char* testFile, SkSLTestFlags flags) {
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
    // We can't clone elements without a valid ThreadContext.
    SkSL::ThreadContext::Start(&compiler, SkSL::ProgramKind::kFragment, settings);
    for (const std::unique_ptr<SkSL::ProgramElement>& element : program->fOwnedElements) {
        std::string original = element->description();
        std::string cloned = element->clone()->description();
        REPORTER_ASSERT(r, original == cloned,
                "Mismatch after clone!\nOriginal: %s\nCloned: %s\n", original.c_str(),
                cloned.c_str());
    }
    SkSL::ThreadContext::End();
}

static void report_rp_pass(skiatest::Reporter* r, const char* testFile, SkSLTestFlags flags) {
    if (!(flags & SkSLTestFlag::CPU) && !(flags & SkSLTestFlag::ES3)) {
        ERRORF(r, "NEW: %s", testFile);
    }
}

static void report_rp_fail(skiatest::Reporter* r,
                           const char* testFile,
                           SkSLTestFlags flags,
                           const char* reason) {
    if ((flags & SkSLTestFlag::CPU) || (flags & SkSLTestFlag::ES3)) {
        ERRORF(r, "%s: %s", testFile, reason);
    }
}

static void test_raster_pipeline(skiatest::Reporter* r,
                                 const char* testFile,
                                 SkSLTestFlags flags) {
    SkString shaderString = load_source(r, testFile, "");
    if (shaderString.isEmpty()) {
        return;
    }

    // In Raster Pipeline, we can compile and run test shaders directly, without involving a surface
    // at all.
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;
    settings.fMaxVersionAllowed = SkSL::Version::k300;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::ProgramKind::kRuntimeShader, shaderString.c_str(), settings);
    if (!program) {
        ERRORF(r, "%s: Unexpected compilation error\n%s", testFile, compiler.errorText().c_str());
        return;
    }
    const SkSL::FunctionDeclaration* main = program->getFunction("main");
    if (!main) {
        ERRORF(r, "%s: Program must have a 'main' function", testFile);
        return;
    }

    // Match up uniforms from the program against our list of test uniforms, and build up a data
    // buffer of uniform floats.
    size_t offset = 0;
    TArray<SkRuntimeEffect::Uniform> uniforms;
    const SkSL::Context& ctx(compiler.context());

    for (const SkSL::ProgramElement* elem : program->elements()) {
        // Variables (uniform, etc.)
        if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl = global.declaration()->as<SkSL::VarDeclaration>();
            const SkSL::Variable& var = *varDecl.var();

            if (var.type().isEffectChild()) {
                ERRORF(r, "%s: Test program cannot contain child effects", testFile);
                return;
            }
            // 'uniform' variables
            if (var.modifierFlags().isUniform()) {
                uniforms.push_back(SkRuntimeEffectPriv::VarAsUniform(var, ctx, &offset));
            }
        }
    }

    TArray<float> uniformValues;
    for (const SkRuntimeEffect::Uniform& programUniform : uniforms) {
        bool foundMatch = false;
        for (const UniformData& data : kUniformData) {
            if (data.name == programUniform.name) {
                SkASSERT(data.span.size() * sizeof(float) == programUniform.sizeInBytes());
                foundMatch = true;
                uniformValues.push_back_n(data.span.size(), data.span.data());
                break;
            }
        }
        if (!foundMatch) {
            report_rp_fail(r, testFile, flags, "unsupported uniform");
            return;
        }
    }

    // Compile our program.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    SkRasterPipeline pipeline(&alloc);
    SkSL::DebugTracePriv debugTrace;
    std::unique_ptr<SkSL::RP::Program> rasterProg =
            SkSL::MakeRasterPipelineProgram(*program,
                                            *main->definition(),
                                            &debugTrace);
    if (!rasterProg) {
        report_rp_fail(r, testFile, flags, "code is not supported");
        return;
    }

    // Append the SkSL program to the raster pipeline.
    pipeline.append_constant_color(&alloc, SkColors::kTransparent);
    rasterProg->appendStages(&pipeline, &alloc, /*callbacks=*/nullptr, SkSpan(uniformValues));

    // Move the float values from RGBA into an 8888 memory buffer.
    uint32_t out[SkRasterPipeline_kMaxStride_highp] = {};
    SkRasterPipeline_MemoryCtx outCtx{/*pixels=*/out, /*stride=*/SkRasterPipeline_kMaxStride_highp};
    pipeline.append(SkRasterPipelineOp::store_8888, &outCtx);
    pipeline.run(0, 0, 1, 1);

    // Make sure the first pixel (exclusively) of `out` is green. If the program compiled
    // successfully, we expect it to run without error, and will assert if it doesn't.
    uint32_t expected = 0xFF00FF00;
    if (out[0] != expected) {
        ERRORF(r, "%s: Raster Pipeline failed. Expected solid green, got ARGB:%02X%02X%02X%02X",
                  testFile,
                  (out[0] >> 24) & 0xFF,
                  (out[0] >> 16) & 0xFF,
                  (out[0] >> 8) & 0xFF,
                  out[0] & 0xFF);
        return;
    }

    // Success!
    report_rp_pass(r, testFile, flags);
}


#if defined(SK_GANESH)
#define DEF_GANESH_SKSL_TEST(flags, ctsEnforcement, name, path) \
    DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkSL##name##_Ganesh, \
                                                       r,                   \
                                                       ctxInfo,             \
                                                       is_gpu(flags),       \
                                                       ctsEnforcement) {    \
        test_ganesh(r, ctxInfo.directContext(), path, flags);               \
    }
#else
#define DEF_GANESH_SKSL_TEST(flags, ctsEnforcement, name, path) /* Ganesh is disabled */
#endif

#if defined(SK_GRAPHITE)
static bool is_native_context_or_dawn(skgpu::ContextType type) {
    // This avoids re-testing Dawn over and over again against every possible API.
    return skgpu::IsNativeBackend(type) ||
           type == skgpu::ContextType::kDawn;
}

#define DEF_GRAPHITE_SKSL_TEST(flags, ctsEnforcement, name, path)         \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkSL##name##_Graphite,     \
                                               is_native_context_or_dawn, \
                                               r,                         \
                                               context,                   \
                                               /*opt_filter=*/nullptr,    \
                                               is_gpu(flags),             \
                                               ctsEnforcement) {          \
        test_graphite(r, context, path, flags);                           \
    }
#else
#define DEF_GRAPHITE_SKSL_TEST(flags, ctsEnforcement, name, path) /* Graphite is disabled */
#endif

#define SKSL_TEST(flags, ctsEnforcement, name, path)                                       \
    DEF_CONDITIONAL_TEST(SkSL##name##_CPU, r, is_cpu(flags)) { test_cpu(r, path, flags); } \
    DEF_TEST(SkSL##name##_RP, r) { test_raster_pipeline(r, path, flags); }                 \
    DEF_TEST(SkSL##name##_Clone, r) { test_clone(r, path, flags); }                        \
    DEF_GANESH_SKSL_TEST(flags, ctsEnforcement, name, path)                                \
    DEF_GRAPHITE_SKSL_TEST(flags, ctsEnforcement, name, path)

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

constexpr SkSLTestFlags CPU = SkSLTestFlag::CPU;
constexpr SkSLTestFlags ES3 = SkSLTestFlag::ES3;
constexpr SkSLTestFlags GPU = SkSLTestFlag::GPU;
constexpr SkSLTestFlags GPU_ES3 = SkSLTestFlag::GPU_ES3;
constexpr SkSLTestFlags UsesNaN = SkSLTestFlag::UsesNaN;
constexpr auto kApiLevel_T = CtsEnforcement::kApiLevel_T;
constexpr auto kApiLevel_U = CtsEnforcement::kApiLevel_U;
constexpr auto kNever = CtsEnforcement::kNever;
[[maybe_unused]] constexpr auto kNextRelease = CtsEnforcement::kNextRelease;

SKSL_TEST(ES3 | GPU_ES3, kApiLevel_T, ArrayFolding,                    "folding/ArrayFolding.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ArraySizeFolding,                "folding/ArraySizeFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, AssignmentOps,                   "folding/AssignmentOps.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, BoolFolding,                     "folding/BoolFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, CastFolding,                     "folding/CastFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntFoldingES2,                   "folding/IntFoldingES2.rts")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntFoldingES3,                   "folding/IntFoldingES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FloatFolding,                    "folding/FloatFolding.rts")
SKSL_TEST(CPU | GPU,     kNextRelease,LogicalNot,                      "folding/LogicalNot.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, MatrixFoldingES2,                "folding/MatrixFoldingES2.rts")
SKSL_TEST(ES3 | GPU_ES3, kNever,      MatrixFoldingES3,                "folding/MatrixFoldingES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, MatrixNoOpFolding,               "folding/MatrixNoOpFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, MatrixScalarNoOpFolding,         "folding/MatrixScalarNoOpFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, MatrixVectorNoOpFolding,         "folding/MatrixVectorNoOpFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Negation,                        "folding/Negation.rts")
// TODO(skia:13035): This test fails on Nvidia GPUs on OpenGL but passes Vulkan. Re-enable the test
// on Vulkan when granular GPU backend selection is supported.
SKSL_TEST(CPU,           kApiLevel_T, PreserveSideEffects,             "folding/PreserveSideEffects.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SelfAssignment,                  "folding/SelfAssignment.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ShortCircuitBoolFolding,         "folding/ShortCircuitBoolFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, StructFieldFolding,              "folding/StructFieldFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, StructFieldNoFolding,            "folding/StructFieldNoFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwitchCaseFolding,               "folding/SwitchCaseFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleFolding,                  "folding/SwizzleFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, TernaryFolding,                  "folding/TernaryFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, VectorScalarFolding,             "folding/VectorScalarFolding.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, VectorVectorFolding,             "folding/VectorVectorFolding.rts")

SKSL_TEST(CPU | GPU,     kNextRelease,CommaExpressionsAllowInlining,                    "inliner/CommaExpressionsAllowInlining.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      DoWhileBodyMustBeInlinedIntoAScope,               "inliner/DoWhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      DoWhileTestCannotBeInlined,                       "inliner/DoWhileTestCannotBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ForBodyMustBeInlinedIntoAScope,                   "inliner/ForBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ForInitializerExpressionsCanBeInlined,            "inliner/ForInitializerExpressionsCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ForWithoutReturnInsideCanBeInlined,               "inliner/ForWithoutReturnInsideCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ForWithReturnInsideCannotBeInlined,               "inliner/ForWithReturnInsideCannotBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IfBodyMustBeInlinedIntoAScope,                    "inliner/IfBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IfElseBodyMustBeInlinedIntoAScope,                "inliner/IfElseBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IfElseChainWithReturnsCanBeInlined,               "inliner/IfElseChainWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IfTestCanBeInlined,                               "inliner/IfTestCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IfWithReturnsCanBeInlined,                        "inliner/IfWithReturnsCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineKeywordOverridesThreshold,                  "inliner/InlineKeywordOverridesThreshold.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlinerAvoidsVariableNameOverlap,                 "inliner/InlinerAvoidsVariableNameOverlap.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlinerElidesTempVarForReturnsInsideBlock,        "inliner/InlinerElidesTempVarForReturnsInsideBlock.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlinerUsesTempVarForMultipleReturns,             "inliner/InlinerUsesTempVarForMultipleReturns.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlinerUsesTempVarForReturnsInsideBlockWithVar,   "inliner/InlinerUsesTempVarForReturnsInsideBlockWithVar.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineThreshold,                                  "inliner/InlineThreshold.sksl")
SKSL_TEST(ES3 | GPU_ES3, kApiLevel_U, InlineUnscopedVariable,                           "inliner/InlineUnscopedVariable.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineWithModifiedArgument,                       "inliner/InlineWithModifiedArgument.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineWithNestedBigCalls,                         "inliner/InlineWithNestedBigCalls.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineWithUnmodifiedArgument,                     "inliner/InlineWithUnmodifiedArgument.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InlineWithUnnecessaryBlocks,                      "inliner/InlineWithUnnecessaryBlocks.sksl")
// TODO(tint:1932): GPU is disabled for now; test exposes a bug in the Tint SPIR-V Reader
SKSL_TEST(CPU,           kNextRelease,IntrinsicNameCollision,                           "inliner/IntrinsicNameCollision.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,ModifiedArrayParametersCannotBeInlined,           "inliner/ModifiedArrayParametersCannotBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,ModifiedStructParametersCannotBeInlined,          "inliner/ModifiedStructParametersCannotBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, NoInline,                                         "inliner/NoInline.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ShortCircuitEvaluationsCannotInlineRightHandSide, "inliner/ShortCircuitEvaluationsCannotInlineRightHandSide.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      StaticSwitchInline,                               "inliner/StaticSwitch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, StructsCanBeInlinedSafely,                        "inliner/StructsCanBeInlinedSafely.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleCanBeInlinedDirectly,                      "inliner/SwizzleCanBeInlinedDirectly.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TernaryResultsCannotBeInlined,                    "inliner/TernaryResultsCannotBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TernaryTestCanBeInlined,                          "inliner/TernaryTestCanBeInlined.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TrivialArgumentsInlineDirectly,                   "inliner/TrivialArgumentsInlineDirectly.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      TrivialArgumentsInlineDirectlyES3,                "inliner/TrivialArgumentsInlineDirectlyES3.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      WhileBodyMustBeInlinedIntoAScope,                 "inliner/WhileBodyMustBeInlinedIntoAScope.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      WhileTestCannotBeInlined,                         "inliner/WhileTestCannotBeInlined.sksl")

SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicAbsFloat,               "intrinsics/AbsFloat.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicAbsInt,                 "intrinsics/AbsInt.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicAny,                    "intrinsics/Any.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicAll,                    "intrinsics/All.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicCeil,                   "intrinsics/Ceil.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicClampInt,               "intrinsics/ClampInt.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicClampUInt,              "intrinsics/ClampUInt.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicClampFloat,             "intrinsics/ClampFloat.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicCross,                  "intrinsics/Cross.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicDegrees,                "intrinsics/Degrees.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicDeterminant,            "intrinsics/Determinant.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicDFdx,                   "intrinsics/DFdx.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicDFdy,                   "intrinsics/DFdy.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicDot,                    "intrinsics/Dot.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicFract,                  "intrinsics/Fract.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicFloatBitsToInt,         "intrinsics/FloatBitsToInt.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicFloatBitsToUint,        "intrinsics/FloatBitsToUint.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicFloor,                  "intrinsics/Floor.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicFwidth,                 "intrinsics/Fwidth.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicIntBitsToFloat,         "intrinsics/IntBitsToFloat.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicIsInf,                  "intrinsics/IsInf.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicLength,                 "intrinsics/Length.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicMatrixCompMultES2,      "intrinsics/MatrixCompMultES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMatrixCompMultES3,      "intrinsics/MatrixCompMultES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicMaxFloat,               "intrinsics/MaxFloat.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMaxInt,                 "intrinsics/MaxInt.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMaxUint,                "intrinsics/MaxUint.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicMinFloat,               "intrinsics/MinFloat.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMinInt,                 "intrinsics/MinInt.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMinUint,                "intrinsics/MinUint.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicMixFloatES2,            "intrinsics/MixFloatES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicMixFloatES3,            "intrinsics/MixFloatES3.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicModf,                   "intrinsics/Modf.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicNot,                    "intrinsics/Not.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicOuterProduct,           "intrinsics/OuterProduct.sksl")
// Fails on Mac OpenGL + Radeon 5300M (skia:12434)
// SKSL_TEST(GPU_ES3,    kNever,      IntrinsicPackUnorm2x16,          "intrinsics/PackUnorm2x16.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicRadians,                "intrinsics/Radians.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicRound,                  "intrinsics/Round.sksl")
SKSL_TEST(GPU_ES3,       kNever,      IntrinsicRoundEven,              "intrinsics/RoundEven.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicSaturate,               "intrinsics/Saturate.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicSignFloat,              "intrinsics/SignFloat.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicSignInt,                "intrinsics/SignInt.sksl")
SKSL_TEST(CPU | GPU,     kNever,      IntrinsicSqrt,                   "intrinsics/Sqrt.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, IntrinsicStep,                   "intrinsics/Step.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicTrunc,                  "intrinsics/Trunc.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicTranspose,              "intrinsics/Transpose.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      IntrinsicUintBitsToFloat,        "intrinsics/UintBitsToFloat.sksl")

SKSL_TEST(ES3 | GPU_ES3, kNever,      ArrayNarrowingConversions,       "runtime/ArrayNarrowingConversions.rts")
SKSL_TEST(ES3 | GPU_ES3, kNever,      Commutative,                     "runtime/Commutative.rts")
SKSL_TEST(CPU,           kNever,      DivideByZero,                    "runtime/DivideByZero.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, LoopFloat,                       "runtime/LoopFloat.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, LoopInt,                         "runtime/LoopInt.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_U, Ossfuzz52603,                    "runtime/Ossfuzz52603.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, QualifierOrder,                  "runtime/QualifierOrder.rts")
SKSL_TEST(CPU | GPU,     kApiLevel_T, PrecisionQualifiers,             "runtime/PrecisionQualifiers.rts")

SKSL_TEST(ES3 | GPU_ES3 | UsesNaN, kNever, RecursiveComparison_Arrays,  "runtime/RecursiveComparison_Arrays.rts")
SKSL_TEST(ES3 | GPU_ES3 | UsesNaN, kNever, RecursiveComparison_Structs, "runtime/RecursiveComparison_Structs.rts")
SKSL_TEST(ES3 | GPU_ES3 | UsesNaN, kNever, RecursiveComparison_Types,   "runtime/RecursiveComparison_Types.rts")
SKSL_TEST(ES3 | GPU_ES3 | UsesNaN, kNever, RecursiveComparison_Vectors, "runtime/RecursiveComparison_Vectors.rts")

SKSL_TEST(ES3 | GPU_ES3, kNever,      ArrayCast,                       "shared/ArrayCast.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ArrayComparison,                 "shared/ArrayComparison.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ArrayConstructors,               "shared/ArrayConstructors.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,ArrayFollowedByScalar,           "shared/ArrayFollowedByScalar.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ArrayTypes,                      "shared/ArrayTypes.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Assignment,                      "shared/Assignment.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, CastsRoundTowardZero,            "shared/CastsRoundTowardZero.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, CommaMixedTypes,                 "shared/CommaMixedTypes.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, CommaSideEffects,                "shared/CommaSideEffects.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, CompileTimeConstantVariables,    "shared/CompileTimeConstantVariables.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ConstantCompositeAccessViaConstantIndex, "shared/ConstantCompositeAccessViaConstantIndex.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ConstantCompositeAccessViaDynamicIndex,  "shared/ConstantCompositeAccessViaDynamicIndex.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ConstantIf,                      "shared/ConstantIf.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ConstArray,                      "shared/ConstArray.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ConstVariableComparison,         "shared/ConstVariableComparison.sksl")
SKSL_TEST(CPU | GPU,     kNever,      DeadGlobals,                     "shared/DeadGlobals.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      DeadLoopVariable,                "shared/DeadLoopVariable.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DeadIfStatement,                 "shared/DeadIfStatement.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DeadReturn,                      "shared/DeadReturn.sksl")
// TODO(skia:12012): some Radeons crash when compiling this code; disable them.
SKSL_TEST(ES3/* | GPU_ES3*/,kNever,   DeadReturnES3,                   "shared/DeadReturnES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DeadStripFunctions,              "shared/DeadStripFunctions.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DependentInitializers,           "shared/DependentInitializers.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, DoubleNegation,                  "shared/DoubleNegation.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      DoWhileControlFlow,              "shared/DoWhileControlFlow.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, EmptyBlocksES2,                  "shared/EmptyBlocksES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      EmptyBlocksES3,                  "shared/EmptyBlocksES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ForLoopControlFlow,              "shared/ForLoopControlFlow.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionAnonymousParameters,     "shared/FunctionAnonymousParameters.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionArgTypeMatch,            "shared/FunctionArgTypeMatch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionReturnTypeMatch,         "shared/FunctionReturnTypeMatch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Functions,                       "shared/Functions.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionPrototype,               "shared/FunctionPrototype.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, GeometricIntrinsics,             "shared/GeometricIntrinsics.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, HelloWorld,                      "shared/HelloWorld.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Hex,                             "shared/Hex.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      HexUnsigned,                     "shared/HexUnsigned.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InoutParameters,                 "shared/InoutParameters.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, InoutParamsAreDistinct,          "shared/InoutParamsAreDistinct.sksl")
SKSL_TEST(ES3 | GPU_ES3, kApiLevel_U, IntegerDivisionES3,              "shared/IntegerDivisionES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, LogicalAndShortCircuit,          "shared/LogicalAndShortCircuit.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, LogicalOrShortCircuit,           "shared/LogicalOrShortCircuit.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Matrices,                        "shared/Matrices.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      MatricesNonsquare,               "shared/MatricesNonsquare.sksl")
// TODO(skia:12443) The MatrixConstructors tests actually don't work on MANY devices. The GLSL SkQP
// suite does a terrible job of enforcing this rule. We still test the ES2 variant on CPU.
SKSL_TEST(CPU,           kNever,      MatrixConstructorsES2,           "shared/MatrixConstructorsES2.sksl")
SKSL_TEST(ES3,           kNever,      MatrixConstructorsES3,           "shared/MatrixConstructorsES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, MatrixEquality,                  "shared/MatrixEquality.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,MatrixIndexLookup,               "shared/MatrixIndexLookup.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,MatrixIndexStore,                "shared/MatrixIndexStore.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, MatrixOpEqualsES2,               "shared/MatrixOpEqualsES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kApiLevel_U, MatrixOpEqualsES3,               "shared/MatrixOpEqualsES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, MatrixScalarMath,                "shared/MatrixScalarMath.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,MatrixSwizzleStore,              "shared/MatrixSwizzleStore.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, MatrixToVectorCast,              "shared/MatrixToVectorCast.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, MultipleAssignments,             "shared/MultipleAssignments.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, NumberCasts,                     "shared/NumberCasts.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,NestedComparisonIntrinsics,      "shared/NestedComparisonIntrinsics.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, OperatorsES2,                    "shared/OperatorsES2.sksl")
SKSL_TEST(GPU_ES3,       kNever,      OperatorsES3,                    "shared/OperatorsES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Ossfuzz36852,                    "shared/Ossfuzz36852.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, OutParams,                       "shared/OutParams.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, OutParamsAreDistinct,            "shared/OutParamsAreDistinct.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, OutParamsAreDistinctFromGlobal,  "shared/OutParamsAreDistinctFromGlobal.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      OutParamsFunctionCallInArgument, "shared/OutParamsFunctionCallInArgument.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, OutParamsDoubleSwizzle,          "shared/OutParamsDoubleSwizzle.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,PostfixExpressions,              "shared/PostfixExpressions.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,PrefixExpressionsES2,            "shared/PrefixExpressionsES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      PrefixExpressionsES3,            "shared/PrefixExpressionsES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ResizeMatrix,                    "shared/ResizeMatrix.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ResizeMatrixNonsquare,           "shared/ResizeMatrixNonsquare.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ReturnsValueOnEveryPathES2,      "shared/ReturnsValueOnEveryPathES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ReturnsValueOnEveryPathES3,      "shared/ReturnsValueOnEveryPathES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ScalarConversionConstructorsES2, "shared/ScalarConversionConstructorsES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ScalarConversionConstructorsES3, "shared/ScalarConversionConstructorsES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ScopedSymbol,                    "shared/ScopedSymbol.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, StackingVectorCasts,             "shared/StackingVectorCasts.sksl")
SKSL_TEST(CPU | GPU_ES3, kNever,      StaticSwitch,                    "shared/StaticSwitch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, StructArrayFollowedByScalar,     "shared/StructArrayFollowedByScalar.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,StructIndexLookup,               "shared/StructIndexLookup.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,StructIndexStore,                "shared/StructIndexStore.sksl")
// TODO(skia:13920): StructComparison currently exposes a bug in SPIR-V codegen.
SKSL_TEST(ES3,           kNextRelease,StructComparison,                "shared/StructComparison.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, StructsInFunctions,              "shared/StructsInFunctions.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Switch,                          "shared/Switch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwitchDefaultOnly,               "shared/SwitchDefaultOnly.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwitchWithFallthrough,           "shared/SwitchWithFallthrough.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwitchWithFallthroughAndVarDecls,"shared/SwitchWithFallthroughAndVarDecls.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwitchWithLoops,                 "shared/SwitchWithLoops.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      SwitchWithLoopsES3,              "shared/SwitchWithLoopsES3.sksl")
SKSL_TEST(CPU | GPU,     kNever,      SwizzleAsLValue,                 "shared/SwizzleAsLValue.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      SwizzleAsLValueES3,              "shared/SwizzleAsLValueES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleBoolConstants,            "shared/SwizzleBoolConstants.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleByConstantIndex,          "shared/SwizzleByConstantIndex.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      SwizzleByIndex,                  "shared/SwizzleByIndex.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleConstants,                "shared/SwizzleConstants.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,SwizzleIndexLookup,              "shared/SwizzleIndexLookup.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,SwizzleIndexStore,               "shared/SwizzleIndexStore.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleLTRB,                     "shared/SwizzleLTRB.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleOpt,                      "shared/SwizzleOpt.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleScalar,                   "shared/SwizzleScalar.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleScalarBool,               "shared/SwizzleScalarBool.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, SwizzleScalarInt,                "shared/SwizzleScalarInt.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,TemporaryIndexLookup,            "shared/TemporaryIndexLookup.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TernaryAsLValueEntirelyFoldable, "shared/TernaryAsLValueEntirelyFoldable.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TernaryAsLValueFoldableTest,     "shared/TernaryAsLValueFoldableTest.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,TernaryComplexNesting,           "shared/TernaryComplexNesting.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, TernaryExpression,               "shared/TernaryExpression.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,TernaryNesting,                  "shared/TernaryNesting.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, TernarySideEffects,              "shared/TernarySideEffects.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, UnaryPositiveNegative,           "shared/UnaryPositiveNegative.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, UniformArray,                    "shared/UniformArray.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, UniformMatrixResize,             "shared/UniformMatrixResize.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, UnusedVariables,                 "shared/UnusedVariables.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, VectorConstructors,              "shared/VectorConstructors.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, VectorToMatrixCast,              "shared/VectorToMatrixCast.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, VectorScalarMath,                "shared/VectorScalarMath.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      WhileLoopControlFlow,            "shared/WhileLoopControlFlow.sksl")

SKSL_TEST(CPU | GPU,     kNextRelease,VoidInSequenceExpressions,       "workarounds/VoidInSequenceExpressions.sksl")
