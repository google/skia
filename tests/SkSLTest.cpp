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
#include "src/base/SkStringView.h"
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
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <regex>
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
#include "tools/graphite/GraphiteTestContext.h"
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

static bool failure_is_expected(std::string_view deviceName,    // "Geforce RTX4090"
                                std::string_view backendAPI,    // "OpenGL"
                                std::string_view name,          // "MatrixToVectorCast"
                                skiatest::TestType testType) {  // skiatest::TestType::kGraphite
    enum TestTypeMatcher { CPU, Ganesh, Graphite, GPU /* either Ganesh or Graphite */ };

    struct TestDisable {
        std::optional<std::regex> deviceName;
        std::optional<std::string_view> backendAPI;
        std::optional<TestTypeMatcher> testTypeMatcher;
        std::optional<bool> platform;
    };

    using TestDisableMap = THashMap<std::string_view, std::vector<TestDisable>>;

    // TODO(b/40044139): migrate test-disable list from dm_flags into this map
    static SkNoDestructor<TestDisableMap> testDisableMap{[] {
        #define ADRENO "Adreno \\(TM\\) "
        #define NVIDIA "(Tegra|Quadro|RTX|GTX) "

        TestDisableMap disables;
        constexpr std::nullopt_t _ = std::nullopt;
        using regex = std::regex;

#if defined(SK_BUILD_FOR_UNIX)
        constexpr bool kLinux = true;
#else
        constexpr bool kLinux = false;
#endif
#if defined(SK_BUILD_FOR_MAC)
        constexpr bool kMac = true;
#else
        constexpr bool kMac = false;
#endif
#if defined(SK_BUILD_FOR_IOS)
        constexpr bool kiOS = true;
#else
        constexpr bool kiOS = false;
#endif
#if defined(SK_BUILD_FOR_WIN)
        constexpr bool kWindows = true;
#else
        constexpr bool kWindows = false;
#endif
#if defined(SK_BUILD_FOR_ANDROID)
        constexpr bool kAndroid = true;
#else
        constexpr bool kAndroid = false;
#endif

        // - Apple --------------------------------------------------------------------------------
        // MacOS/iOS do not handle short-circuit evaluation properly in OpenGL (chromium:307751)
        for (const char* test : {"LogicalAndShortCircuit",
                                 "LogicalOrShortCircuit"}) {
            disables[test].push_back({_, "OpenGL", GPU, kMac || kiOS});
        }

        // ANGLE has a handful of Mac-specific bugs.
        for (const char* test : {"MatrixScalarNoOpFolding",         // anglebug.com/7525
                                 "MatrixScalarMath",                // anglebug.com/7525
                                 "SwizzleIndexStore",               // Apple bug FB12055941
                                 "OutParamsAreDistinctFromGlobal",  // anglebug.com/7145
                                 "IntrinsicMixFloatES3"}) {         // anglebug.com/7245
            disables[test].push_back({_, "ANGLE", GPU, kMac});
        }

        // Switch fallthrough has some issues on iOS.
        disables["SwitchWithFallthrough"].push_back({_, "OpenGL", GPU, kiOS});

        // - ARM ----------------------------------------------------------------------------------
        // Mali 400 is a very old driver its share of quirks, particularly in relation to matrices.
        for (const char* test : {"Matrices",                // b/40043539
                                 "MatrixNoOpFolding",
                                 "MatrixScalarMath",        // b/40043764
                                 "MatrixSwizzleStore",
                                 "MatrixScalarNoOpFolding", // b/40044644
                                 "UnaryPositiveNegative",
                                 "Cross"}) {
            disables[test].push_back({regex("Mali-400"), _, GPU, _});
        }

        // - Nvidia -------------------------------------------------------------------------------
        // Tegra3 has several issues, but the inability to break from a for loop is a common theme.
        for (const char* test : {"Switch",                            // b/40043561
                                 "SwitchDefaultOnly",                 //  "      "
                                 "SwitchWithFallthrough",             //  "      "
                                 "SwitchWithFallthroughAndVarDecls",  //  "      "
                                 "SwitchWithLoops",                   //  "      "
                                 "SwitchCaseFolding",                 //  "      "
                                 "LoopFloat",                         //  "      "
                                 "LoopInt",                           //  "      "
                                 "MatrixScalarNoOpFolding",           // b/40044644
                                 "MatrixScalarMath",                  // b/40043764
                                 "MatrixFoldingES2",                  // b/40043017
                                 "MatrixEquality",                    // b/40043017
                                 "IntrinsicFract",
                                 "ModifiedStructParametersCannotBeInlined"}) {
            disables[test].push_back({regex("Tegra 3"), _, GPU, _});
        }

        // Various Nvidia GPUs generate errors when assembling weird matrices, and erroneously
        // constant-fold expressions with side-effects in constructors when compiling GLSL.
        for (const char* test : {"MatrixConstructorsES2",    // b/40043524
                                 "MatrixConstructorsES3",    // b/40043524
                                 "MatrixScalarNoOpFolding",  // b/40044644
                                 "PreserveSideEffects",      // b/40044140
                                 "StructFieldNoFolding"}) {  // b/40044479
            disables[test].push_back({regex(NVIDIA), "OpenGL", _, _});
            disables[test].push_back({regex(NVIDIA), "ANGLE GL", _, _});
        }

        disables["IntrinsicMixFloatES3"].push_back({regex("RTX "), "Vulkan", _, kWindows});

        // The Golo features P400s with older and buggier drivers than usual.
        for (const char* test : {"PreserveSideEffects",  // b/40044140
                                 "CommaSideEffects"}) {
            disables[test].push_back({regex("Quadro P400"), _, _, kLinux});
        }

        // b/318725123
        for (const char* test : {"UniformArray",
                                 "TemporaryIndexLookup",
                                 "MatrixIndexLookup"}) {
            disables[test].push_back({regex("Quadro P400"), "Dawn Vulkan", Graphite, kWindows});
        }

        // - PowerVR ------------------------------------------------------------------------------
        for (const char* test : {"OutParamsAreDistinct",              // b/40044222
                                 "OutParamsAreDistinctFromGlobal"}) {
            disables[test].push_back({regex("PowerVR Rogue GE8300"), _, GPU, _});
        }

        // - Radeon -------------------------------------------------------------------------------
        for (const char* test : {"DeadReturnES3",              // b/301326132
                                 "IntrinsicAll",               // b/40045114
                                 "MatrixConstructorsES3",      // b/40043524
                                 "MatrixScalarNoOpFolding",    // b/40044644
                                 "StructIndexStore",           // b/40045236
                                 "SwizzleIndexLookup",         // b/40045254
                                 "SwizzleIndexStore"}) {       // b/40045254
            disables[test].push_back({regex("Radeon.*(R9|HD)"), "OpenGL", GPU, _});
            disables[test].push_back({regex("Radeon.*(R9|HD)"), "ANGLE GL", GPU, _});
        }

        // The Radeon Vega 6 doesn't return zero for the derivative of a uniform.
        for (const char* test : {"IntrinsicDFdy",
                                 "IntrinsicDFdx",
                                 "IntrinsicFwidth"}) {
            disables[test].push_back({regex("AMD RADV RENOIR"), _, GPU, _});
        }

        // - Adreno -------------------------------------------------------------------------------
        // Disable broken tests on Android with Adreno GPUs (b/40043413, b/40045254)
        for (const char* test : {"ArrayCast",
                                 "ArrayComparison",
                                 "CommaSideEffects",
                                 "IntrinsicMixFloatES2",
                                 "IntrinsicClampFloat",
                                 "SwitchWithFallthrough",
                                 "SwizzleIndexLookup",
                                 "SwizzleIndexStore"}) {
            disables[test].push_back({regex(ADRENO "[3456]"), _, _, kAndroid});
        }

        // Older Adreno 5/6xx drivers report a pipeline error or silently fail when handling inouts.
        for (const char* test : {"VoidInSequenceExpressions",  // b/295217166
                                 "InoutParameters",            // b/40043966
                                 "OutParams",
                                 "OutParamsDoubleSwizzle",
                                 "OutParamsNoInline",
                                 "OutParamsFunctionCallInArgument"}) {
            disables[test].push_back({regex(ADRENO "[56]"), "Vulkan", _, kAndroid});
        }

        for (const char* test : {"MatrixToVectorCast",     // b/40043288
                                 "StructsInFunctions"}) {  // b/40043024
            disables[test].push_back({regex(ADRENO "[345]"), "OpenGL", _, kAndroid});
        }

        // Constructing a matrix from vectors and scalars can be surprisingly finicky (b/40043539)
        for (const char* test : {"Matrices",
                                 "MatrixNoOpFolding"}) {
            disables[test].push_back({regex(ADRENO "3"), "OpenGL", _, kAndroid});
        }

        // Adreno 600 doesn't handle isinf() in OpenGL correctly. (b/40043464)
        disables["IntrinsicIsInf"].push_back({regex(ADRENO "6"), "OpenGL", _, kAndroid});

        // Older Adreno drivers crash when presented with an empty block (b/40044390)
        disables["EmptyBlocksES3"].push_back({regex(ADRENO "(540|630)"), _, _, kAndroid});

        // Adrenos alias out-params to globals improperly (b/40044222)
        disables["OutParamsAreDistinctFromGlobal"].push_back({regex(ADRENO "[3456]"), "OpenGL",
                                                              _, kAndroid});
        // Adreno generates the wrong result for this test. (b/40044477)
        disables["StructFieldFolding"].push_back({regex(ADRENO "[56]"), "OpenGL",
                                                        _, kAndroid});

        // b/318726662
        for (const char* test : {"PrefixExpressionsES2",
                                 "MatrixToVectorCast",
                                 "MatrixConstructorsES2"}) {
            disables[test].push_back({regex(ADRENO "620"), "Vulkan", Graphite, kAndroid});
        }

        // - Intel --------------------------------------------------------------------------------
        // Disable various tests on Intel.
        // Intrinsic floor() on Intel + ANGLE + DirectX is broken (anglebug.com/5588)
        disables["IntrinsicFloor"].push_back({regex("Intel.*(Iris|HD)"), "ANGLE D3D", _, _});

        // Intrinsic not() and mix() are broken on Intel GPUs in Metal. (b/40045105)
        for (const char* test : {"IntrinsicNot",
                                 "IntrinsicMixFloatES3"}) {
            disables[test].push_back({regex("Intel.*(Iris|6000)"), "Metal", _, kMac});
        }

        // Swizzled-index store is broken across many Intel GPUs. (b/40045254)
        disables["SwizzleIndexStore"].push_back({regex("Intel"), "OpenGL", _, kMac});
        disables["SwizzleIndexStore"].push_back({regex("Intel.*Iris"), _, _, kWindows});

        // vec4(mat2) conversions can lead to a crash on Intel + ANGLE (b/40043275)
        for (const char* test : {"VectorToMatrixCast",
                                 "VectorScalarMath",
                                 "TrivialArgumentsInlineDirectly"}) {
            disables[test].push_back({regex("Intel"), "ANGLE", _, kWindows});
        }

        for (const char* test : {"MatrixFoldingES2",
                                 "MatrixEquality",
                                 "TemporaryIndexLookup",  // b/40045228
                                 "SwizzleIndexLookup"}) { // b/40045254
            disables[test].push_back({regex("Intel.*(Iris|4400)"), "OpenGL", _, kWindows});
            disables[test].push_back({regex("Intel.*(Iris|4400)"), "ANGLE",  _, kWindows});
        }

        for (const char* test : {"ReturnsValueOnEveryPathES3",      // b/40043548
                                 "OutParamsAreDistinctFromGlobal",  // b/40044222
                                 "StructFieldFolding"}) {           // b/40044477
            disables[test].push_back({regex("Intel"), "OpenGL", _, kWindows});
            disables[test].push_back({regex("Intel"), "ANGLE GL", _, kWindows});
        }

        for (const char* test : {"SwitchDefaultOnly",               // b/40043548
                                 "ReturnsValueOnEveryPathES3"}) {   // b/40045205
            disables[test].push_back({regex("Intel"), "Vulkan", _, kLinux});
        }

        for (const char* test : {"SwitchDefaultOnly"}) {
            disables[test].push_back({regex("Intel"), "ANGLE", _, kWindows});
        }

        for (const char* test : {"SwizzleAsLValueES3"}) {  // https://anglebug.com/8260
            disables[test].push_back({regex("Intel"), _, _, kWindows});
            disables[test].push_back({_, "ANGLE", _, kWindows});
        }

        // Some Intel GPUs don't return zero for the derivative of a uniform.
        for (const char* test : {"IntrinsicDFdy",
                                 "IntrinsicDFdx",
                                 "IntrinsicFwidth"}) {
            disables[test].push_back({regex("Intel"), _, GPU, _});
        }

        disables["LoopFloat"].push_back({regex("Intel.*(Iris|6000)"), _, _, kMac});  // b/40043507

        #undef ADRENO
        #undef NVIDIA

        return disables;
    }()};

    if (const std::vector<TestDisable>* testDisables = testDisableMap->find(name)) {
        for (const TestDisable& d : *testDisables) {
            if (d.platform.has_value() && !*d.platform) {
                continue;  // disable applies to a different platform
            }
            if (d.backendAPI.has_value() && !skstd::contains(backendAPI, *d.backendAPI)) {
                continue;  // disable applies to a different backend API
            }
            if (d.deviceName.has_value() &&
                !std::regex_search(deviceName.begin(), deviceName.end(), *d.deviceName)) {
                continue;  // disable applies to a different device
            }
            if (d.testTypeMatcher == CPU && testType != skiatest::TestType::kCPU) {
                continue;  // disable only applies to CPU
            }
            if (d.testTypeMatcher == Ganesh && testType != skiatest::TestType::kGanesh) {
                continue;  // disable only applies to Ganesh
            }
            if (d.testTypeMatcher == Graphite && testType != skiatest::TestType::kGraphite) {
                continue;  // disable only applies to Graphites
            }
            if (d.testTypeMatcher == GPU && testType == skiatest::TestType::kCPU) {
                continue;  // disable only applies to GPU
            }
            // This test was disabled.
            return true;
        }
    }

    // This test was not in our disable list.
    return false;
}

static void test_one_permutation(skiatest::Reporter* r,
                                 std::string_view deviceName,
                                 std::string_view backendAPI,
                                 SkSurface* surface,
                                 const char* name,
                                 const char* testFile,
                                 skiatest::TestType testType,
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
    if (failure_is_expected(deviceName, backendAPI, name, testType)) {
        // Some driver bugs can be catastrophic (e.g. crashing dm entirely), so we don't even try to
        // run a shader if we expect that it might fail.
        SkDebugf("%s: skipped %.*s%s\n", testFile, (int)backendAPI.size(), backendAPI.data(),
                                         permutationSuffix);
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

        SkString message = SkStringPrintf("Expected%s: solid green. Actual output from %.*s using "
                                          "%.*s:\n"
                                          "RRGGBBAA RRGGBBAA\n"
                                          "%02X%02X%02X%02X %02X%02X%02X%02X\n"
                                          "%02X%02X%02X%02X %02X%02X%02X%02X",
                                          permutationSuffix,
                                          (int)deviceName.size(), deviceName.data(),
                                          (int)backendAPI.size(), backendAPI.data(),

                                          SkColorGetR(color[0][0]), SkColorGetG(color[0][0]),
                                          SkColorGetB(color[0][0]), SkColorGetA(color[0][0]),

                                          SkColorGetR(color[0][1]), SkColorGetG(color[0][1]),
                                          SkColorGetB(color[0][1]), SkColorGetA(color[0][1]),

                                          SkColorGetR(color[1][0]), SkColorGetG(color[1][0]),
                                          SkColorGetB(color[1][0]), SkColorGetA(color[1][0]),

                                          SkColorGetR(color[1][1]), SkColorGetG(color[1][1]),
                                          SkColorGetB(color[1][1]), SkColorGetA(color[1][1]));

        ERRORF(r, "%s", message.c_str());
    }
}

static void test_permutations(skiatest::Reporter* r,
                              std::string_view deviceName,
                              std::string_view backendAPI,
                              SkSurface* surface,
                              const char* name,
                              const char* testFile,
                              skiatest::TestType testType,
                              bool strictES2) {
    SkRuntimeEffect::Options options = strictES2 ? SkRuntimeEffect::Options{}
                                                 : SkRuntimeEffectPriv::ES3Options();
    options.forceUnoptimized = false;
    test_one_permutation(r, deviceName, backendAPI, surface, name, testFile, testType, "", options);

    options.forceUnoptimized = true;
    test_one_permutation(r, deviceName, backendAPI, surface, name, testFile, testType,
                         " (Unoptimized)", options);
}

static void test_cpu(skiatest::Reporter* r,
                     const char* name,
                     const char* testFile,
                     SkSLTestFlags flags) {
    SkASSERT(flags & SkSLTestFlag::CPU);

    // Create a raster-backed surface.
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface(SkSurfaces::Raster(info));

    test_permutations(r, "CPU", "SkRP", surface.get(), name, testFile,
                      skiatest::TestType::kCPU, /*strictES2=*/true);
}

#if defined(SK_GANESH)
static void test_ganesh(skiatest::Reporter* r,
                        const sk_gpu_test::ContextInfo& ctxInfo,
                        const char* name,
                        const char* testFile,
                        SkSLTestFlags flags) {
    GrDirectContext *ctx = ctxInfo.directContext();

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
    std::string_view deviceName = ctx->priv().caps()->deviceName();
    std::string_view backendAPI = skgpu::ContextTypeName(ctxInfo.type());

    if (shouldRunGPU) {
        test_permutations(r, deviceName, backendAPI, surface.get(), name, testFile,
                          skiatest::TestType::kGanesh, /*strictES2=*/true);
    }
    if (shouldRunGPU_ES3) {
        test_permutations(r, deviceName, backendAPI, surface.get(), name, testFile,
                          skiatest::TestType::kGanesh, /*strictES2=*/false);
    }
}
#endif

#if defined(SK_GRAPHITE)
// Note: SKSL_TEST sets CTS enforcement API level to max(kApiLevel_V, ctsEnforcement) for Graphite.
static void test_graphite(skiatest::Reporter* r,
                          skgpu::graphite::Context* ctx,
                          skiatest::graphite::GraphiteTestContext* testCtx,
                          const char* name,
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

    // Create a GPU-backed Graphite surface.
    std::unique_ptr<skgpu::graphite::Recorder> recorder = ctx->makeRecorder();

    const SkImageInfo info = SkImageInfo::Make({kWidth, kHeight},
                                                kRGBA_8888_SkColorType,
                                                kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    std::string_view deviceName = ctx->priv().caps()->deviceName();
    std::string_view backendAPI = skgpu::ContextTypeName(testCtx->contextType());

    if (shouldRunGPU) {
        test_permutations(r, deviceName, backendAPI, surface.get(), name, testFile,
                          skiatest::TestType::kGraphite, /*strictES2=*/true);
    }
    if (shouldRunGPU_ES3) {
        test_permutations(r, deviceName, backendAPI, surface.get(), name, testFile,
                          skiatest::TestType::kGraphite, /*strictES2=*/false);
    }
}
#endif

static void test_clone(skiatest::Reporter* r, const char* testFile, SkSLTestFlags flags) {
    SkString shaderString = load_source(r, testFile, "");
    if (shaderString.isEmpty()) {
        return;
    }
    SkSL::ProgramSettings settings;
    // TODO(skia:11209): Can we just put the correct #version in the source files that need this?
    settings.fMaxVersionAllowed = is_strict_es2(flags) ? SkSL::Version::k100 : SkSL::Version::k300;
    SkSL::Compiler compiler;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::ProgramKind::kRuntimeShader, shaderString.c_str(), settings);
    if (!program) {
        ERRORF(r, "%s", compiler.errorText().c_str());
        return;
    }

    // Clone every expression in the program, and ensure that its clone generates the same
    // description as the original.
    class CloneVisitor : public SkSL::ProgramVisitor {
    public:
        CloneVisitor(skiatest::Reporter* r) : fReporter(r) {}

        bool visitExpression(const SkSL::Expression& expr) override {
            std::string original = expr.description();
            std::string cloned = expr.clone()->description();
            REPORTER_ASSERT(fReporter, original == cloned,
                            "Mismatch after clone!\nOriginal: %s\nCloned: %s\n",
                            original.c_str(), cloned.c_str());

            return INHERITED::visitExpression(expr);
        }

        skiatest::Reporter* fReporter;

        using INHERITED = ProgramVisitor;
    };

    CloneVisitor{r}.visit(*program);
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
    SkSL::Compiler compiler;
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
    pipeline.appendConstantColor(&alloc, SkColors::kTransparent);
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
        test_ganesh(r, ctxInfo, #name, path, flags);                        \
    }
#else
#define DEF_GANESH_SKSL_TEST(flags, ctsEnforcement, name, path) /* Ganesh is disabled */
#endif

#if defined(SK_GRAPHITE)
static bool is_native_context_or_dawn(skgpu::ContextType type) {
    return skgpu::IsNativeBackend(type) || skgpu::IsDawnBackend(type);
}

#define DEF_GRAPHITE_SKSL_TEST(flags, ctsEnforcement, name, path)         \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkSL##name##_Graphite,     \
                                               is_native_context_or_dawn, \
                                               r,                         \
                                               context,                   \
                                               testContext,               \
                                               /*opt_filter=*/nullptr,    \
                                               is_gpu(flags),             \
                                               ctsEnforcement) {          \
        test_graphite(r, context, testContext, #name, path, flags);       \
    }
#else
#define DEF_GRAPHITE_SKSL_TEST(flags, ctsEnforcement, name, path) /* Graphite is disabled */
#endif

#define SKSL_TEST(flags, ctsEnforcement, name, path)                                              \
    DEF_CONDITIONAL_TEST(SkSL##name##_CPU, r, is_cpu(flags)) { test_cpu(r, #name, path, flags); } \
    DEF_TEST(SkSL##name##_RP, r) { test_raster_pipeline(r, path, flags); }                        \
    DEF_TEST(SkSL##name##_Clone, r) { test_clone(r, path, flags); }                               \
    DEF_GANESH_SKSL_TEST(flags, ctsEnforcement, name, path)                                       \
    DEF_GRAPHITE_SKSL_TEST(flags, std::max(kApiLevel_V, ctsEnforcement), name, path)

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
[[maybe_unused]] constexpr auto kApiLevel_V = CtsEnforcement::kApiLevel_V;
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
SKSL_TEST(CPU | GPU,     kApiLevel_T, PreserveSideEffects,             "folding/PreserveSideEffects.rts")
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
SKSL_TEST(CPU | GPU,     kNextRelease,IntrinsicNameCollision,                           "inliner/IntrinsicNameCollision.sksl")
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
SKSL_TEST(CPU | GPU,     kNextRelease,TypeShadowing,                                    "inliner/TypeShadowing.sksl")
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
SKSL_TEST(CPU | GPU,     kNextRelease,FunctionParameterAliasingFirst,  "runtime/FunctionParameterAliasingFirst.rts")
SKSL_TEST(CPU | GPU,     kNextRelease,FunctionParameterAliasingSecond, "runtime/FunctionParameterAliasingSecond.rts")
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
SKSL_TEST(ES3 | GPU_ES3, kNever,      DeadReturnES3,                   "shared/DeadReturnES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DeadStripFunctions,              "shared/DeadStripFunctions.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, DependentInitializers,           "shared/DependentInitializers.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, DoubleNegation,                  "shared/DoubleNegation.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      DoWhileControlFlow,              "shared/DoWhileControlFlow.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, EmptyBlocksES2,                  "shared/EmptyBlocksES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      EmptyBlocksES3,                  "shared/EmptyBlocksES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, ForLoopControlFlow,              "shared/ForLoopControlFlow.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      ForLoopMultipleInitES3,          "shared/ForLoopMultipleInitES3.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,ForLoopShadowing,                "shared/ForLoopShadowing.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionAnonymousParameters,     "shared/FunctionAnonymousParameters.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionArgTypeMatch,            "shared/FunctionArgTypeMatch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionReturnTypeMatch,         "shared/FunctionReturnTypeMatch.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Functions,                       "shared/Functions.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, FunctionPrototype,               "shared/FunctionPrototype.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, GeometricIntrinsics,             "shared/GeometricIntrinsics.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, HelloWorld,                      "shared/HelloWorld.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Hex,                             "shared/Hex.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      HexUnsigned,                     "shared/HexUnsigned.sksl")
SKSL_TEST(CPU | GPU,     kNextRelease,IfStatement,                     "shared/IfStatement.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, InoutParameters,                 "shared/InoutParameters.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, InoutParamsAreDistinct,          "shared/InoutParamsAreDistinct.sksl")
SKSL_TEST(ES3 | GPU_ES3, kApiLevel_U, IntegerDivisionES3,              "shared/IntegerDivisionES3.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, LogicalAndShortCircuit,          "shared/LogicalAndShortCircuit.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_U, LogicalOrShortCircuit,           "shared/LogicalOrShortCircuit.sksl")
SKSL_TEST(CPU | GPU,     kApiLevel_T, Matrices,                        "shared/Matrices.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      MatricesNonsquare,               "shared/MatricesNonsquare.sksl")
SKSL_TEST(CPU | GPU,     kNever,      MatrixConstructorsES2,           "shared/MatrixConstructorsES2.sksl")
SKSL_TEST(ES3 | GPU_ES3, kNever,      MatrixConstructorsES3,           "shared/MatrixConstructorsES3.sksl")
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
SKSL_TEST(CPU | GPU,     kNextRelease,TernaryOneZeroOptimization,      "shared/TernaryOneZeroOptimization.sksl")
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
