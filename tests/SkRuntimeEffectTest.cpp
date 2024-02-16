/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "include/sksl/SkSLVersion.h"
#include "src/base/SkStringView.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/sksl/SkSLString.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <thread>
#include <utility>

using namespace skia_private;

class GrRecordingContext;
struct GrContextOptions;
struct SkIPoint;

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Surface_Graphite.h"

struct GraphiteInfo {
    skgpu::graphite::Context* context = nullptr;
    skgpu::graphite::Recorder* recorder = nullptr;
};
#else
struct GraphiteInfo {
    void* context = nullptr;
    void* recorder = nullptr;
};
#endif

void test_invalid_effect(skiatest::Reporter* r, const char* src, const char* expected) {
    auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(src));
    REPORTER_ASSERT(r, !effect);
    REPORTER_ASSERT(r, errorText.contains(expected),
                    "Expected error message to contain \"%s\". Actual message: \"%s\"",
                    expected, errorText.c_str());
}

#define EMPTY_MAIN "half4 main(float2 p) { return half4(0); }"

DEF_TEST(SkRuntimeEffectInvalid_NoInVariables, r) {
    // 'in' variables aren't allowed at all:
    test_invalid_effect(r, "in bool b;"    EMPTY_MAIN, "'in'");
    test_invalid_effect(r, "in float f;"   EMPTY_MAIN, "'in'");
    test_invalid_effect(r, "in float2 v;"  EMPTY_MAIN, "'in'");
    test_invalid_effect(r, "in half3x3 m;" EMPTY_MAIN, "'in'");
}

DEF_TEST(SkRuntimeEffectInvalid_UndefinedFunction, r) {
    test_invalid_effect(r, "half4 missing(); half4 main(float2 p) { return missing(); }",
                           "function 'half4 missing()' is not defined");
}

DEF_TEST(SkRuntimeEffectInvalid_UndefinedMain, r) {
    // Shouldn't be possible to create an SkRuntimeEffect without "main"
    test_invalid_effect(r, "", "main");
}

DEF_TEST(SkRuntimeEffectInvalid_SkCapsDisallowed, r) {
    // sk_Caps is an internal system. It should not be visible to runtime effects
    test_invalid_effect(
            r,
            "half4 main(float2 p) { return sk_Caps.floatIs32Bits ? half4(1) : half4(0); }",
            "name 'sk_Caps' is reserved");
}

DEF_TEST(SkRuntimeEffect_DeadCodeEliminationStackOverflow, r) {
    // Verify that a deeply-nested loop does not cause stack overflow during dead-code elimination.
    auto [effect, errorText] = SkRuntimeEffect::MakeForColorFilter(SkString(R"(
        half4 main(half4 color) {
            half value = color.r;

            for (int a=0; a<10; ++a) { // 10
            for (int b=0; b<10; ++b) { // 100
            for (int c=0; c<10; ++c) { // 1000
            for (int d=0; d<10; ++d) { // 10000
                ++value;
            }}}}

            return value.xxxx;
        }
    )"));
    REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
}

DEF_TEST(SkRuntimeEffectCanDisableES2Restrictions, r) {
    auto test_valid_es3 = [](skiatest::Reporter* r, const char* sksl) {
        SkRuntimeEffect::Options opt = SkRuntimeEffectPriv::ES3Options();
        auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(sksl), opt);
        REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
    };

    test_invalid_effect(r, "float f[2] = float[2](0, 1);" EMPTY_MAIN, "construction of array type");
    test_valid_es3     (r, "float f[2] = float[2](0, 1);" EMPTY_MAIN);
}

DEF_TEST(SkRuntimeEffectCanEnableVersion300, r) {
    auto test_valid = [](skiatest::Reporter* r, const char* sksl) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(sksl));
        REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
    };

    test_invalid_effect(r, "#version 100\nfloat f[2] = float[2](0, 1);" EMPTY_MAIN,
                           "construction of array type");
    test_valid         (r, "#version 300\nfloat f[2] = float[2](0, 1);" EMPTY_MAIN);
}

DEF_TEST(SkRuntimeEffectUniformFlags, r) {
    auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(R"(
        uniform int simple;                      // should have no flags
        uniform float arrayOfOne[1];             // should have kArray_Flag
        uniform float arrayOfMultiple[2];        // should have kArray_Flag
        layout(color) uniform float4 color;      // should have kColor_Flag
        uniform half3 halfPrecisionFloat;        // should have kHalfPrecision_Flag
        layout(color) uniform half4 allFlags[2]; // should have Array | Color | HalfPrecision
    )"  EMPTY_MAIN));
    REPORTER_ASSERT(r, effect, "%s", errorText.c_str());

    SkSpan<const SkRuntimeEffect::Uniform> uniforms = effect->uniforms();
    REPORTER_ASSERT(r, uniforms.size() == 6);

    REPORTER_ASSERT(r, uniforms[0].flags == 0);
    REPORTER_ASSERT(r, uniforms[1].flags == SkRuntimeEffect::Uniform::kArray_Flag);
    REPORTER_ASSERT(r, uniforms[2].flags == SkRuntimeEffect::Uniform::kArray_Flag);
    REPORTER_ASSERT(r, uniforms[3].flags == SkRuntimeEffect::Uniform::kColor_Flag);
    REPORTER_ASSERT(r, uniforms[4].flags == SkRuntimeEffect::Uniform::kHalfPrecision_Flag);
    REPORTER_ASSERT(r, uniforms[5].flags == (SkRuntimeEffect::Uniform::kArray_Flag |
                                             SkRuntimeEffect::Uniform::kColor_Flag |
                                             SkRuntimeEffect::Uniform::kHalfPrecision_Flag));
}

DEF_TEST(SkRuntimeEffectValidation, r) {
    auto es2Effect = SkRuntimeEffect::MakeForShader(SkString("#version 100\n" EMPTY_MAIN)).effect;
    auto es3Effect = SkRuntimeEffect::MakeForShader(SkString("#version 300\n" EMPTY_MAIN)).effect;
    REPORTER_ASSERT(r, es2Effect && es3Effect);

    auto es2Caps = SkCapabilities::RasterBackend();
    REPORTER_ASSERT(r, es2Caps->skslVersion() == SkSL::Version::k100);

    REPORTER_ASSERT(r, SkRuntimeEffectPriv::CanDraw(es2Caps.get(), es2Effect.get()));
    REPORTER_ASSERT(r, !SkRuntimeEffectPriv::CanDraw(es2Caps.get(), es3Effect.get()));
}

DEF_TEST(SkRuntimeEffectForColorFilter, r) {
    // Tests that the color filter factory rejects or accepts certain SkSL constructs
    auto test_valid = [r](const char* sksl) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForColorFilter(SkString(sksl));
        REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
    };

    auto test_invalid = [r](const char* sksl, const char* expected) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForColorFilter(SkString(sksl));
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r,
                        errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected,
                        errorText.c_str());
    };

    // Color filters must use the 'half4 main(half4)' signature. Either color can be float4/vec4
    test_valid("half4  main(half4  c) { return c; }");
    test_valid("float4 main(half4  c) { return c; }");
    test_valid("half4  main(float4 c) { return c; }");
    test_valid("float4 main(float4 c) { return c; }");
    test_valid("vec4   main(half4  c) { return c; }");
    test_valid("half4  main(vec4   c) { return c; }");
    test_valid("vec4   main(vec4   c) { return c; }");

    // Invalid return types
    test_invalid("void  main(half4 c) {}",                "'main' must return");
    test_invalid("half3 main(half4 c) { return c.rgb; }", "'main' must return");

    // Invalid argument types (some are valid as shaders, but not color filters)
    test_invalid("half4 main() { return half4(1); }",           "'main' parameter");
    test_invalid("half4 main(float2 p) { return half4(1); }",   "'main' parameter");
    test_invalid("half4 main(float2 p, half4 c) { return c; }", "'main' parameter");

    // sk_FragCoord should not be available
    test_invalid("half4 main(half4 c) { return sk_FragCoord.xy01; }", "unknown identifier");

    // Sampling a child shader requires that we pass explicit coords
    test_valid("uniform shader child;"
               "half4 main(half4 c) { return child.eval(c.rg); }");

    // Sampling a colorFilter requires a color
    test_valid("uniform colorFilter child;"
               "half4 main(half4 c) { return child.eval(c); }");

    // Sampling a blender requires two colors
    test_valid("uniform blender child;"
               "half4 main(half4 c) { return child.eval(c, c); }");
}

DEF_TEST(SkRuntimeEffectForBlender, r) {
    // Tests that the blender factory rejects or accepts certain SkSL constructs
    auto test_valid = [r](const char* sksl) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForBlender(SkString(sksl));
        REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
    };

    auto test_invalid = [r](const char* sksl, const char* expected) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForBlender(SkString(sksl));
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r,
                        errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected,
                        errorText.c_str());
    };

    // Blenders must use the 'half4 main(half4, half4)' signature. Any mixture of float4/vec4/half4
    // is allowed.
    test_valid("half4  main(half4  s, half4  d) { return s; }");
    test_valid("float4 main(float4 s, float4 d) { return d; }");
    test_valid("float4 main(half4  s, float4 d) { return s; }");
    test_valid("half4  main(float4 s, half4  d) { return d; }");
    test_valid("vec4   main(half4  s, half4  d) { return s; }");
    test_valid("half4  main(vec4   s, vec4   d) { return d; }");
    test_valid("vec4   main(vec4   s, vec4   d) { return s; }");

    // Invalid return types
    test_invalid("void  main(half4 s, half4 d) {}",                "'main' must return");
    test_invalid("half3 main(half4 s, half4 d) { return s.rgb; }", "'main' must return");

    // Invalid argument types (some are valid as shaders/color filters)
    test_invalid("half4 main() { return half4(1); }",                    "'main' parameter");
    test_invalid("half4 main(half4 c) { return c; }",                    "'main' parameter");
    test_invalid("half4 main(float2 p) { return half4(1); }",            "'main' parameter");
    test_invalid("half4 main(float2 p, half4 c) { return c; }",          "'main' parameter");
    test_invalid("half4 main(float2 p, half4 a, half4 b) { return a; }", "'main' parameter");
    test_invalid("half4 main(half4 a, half4 b, half4 c) { return a; }",  "'main' parameter");

    // sk_FragCoord should not be available
    test_invalid("half4 main(half4 s, half4 d) { return sk_FragCoord.xy01; }",
                 "unknown identifier");

    // Sampling a child shader requires that we pass explicit coords
    test_valid("uniform shader child;"
               "half4 main(half4 s, half4 d) { return child.eval(s.rg); }");

    // Sampling a colorFilter requires a color
    test_valid("uniform colorFilter child;"
               "half4 main(half4 s, half4 d) { return child.eval(d); }");

    // Sampling a blender requires two colors
    test_valid("uniform blender child;"
               "half4 main(half4 s, half4 d) { return child.eval(s, d); }");
}

DEF_TEST(SkRuntimeEffectForShader, r) {
    // Tests that the shader factory rejects or accepts certain SkSL constructs
    auto test_valid = [r](const char* sksl, SkRuntimeEffect::Options options = {}) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(sksl), options);
        REPORTER_ASSERT(r, effect, "%s", errorText.c_str());
    };

    auto test_invalid = [r](const char* sksl,
                            const char* expected,
                            SkRuntimeEffect::Options options = {}) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(sksl));
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r,
                        errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected,
                        errorText.c_str());
    };

    // Shaders must use the 'half4 main(float2)' signature
    // Either color can be half4/float4/vec4, but the coords must be float2/vec2
    test_valid("half4  main(float2 p) { return p.xyxy; }");
    test_valid("float4 main(float2 p) { return p.xyxy; }");
    test_valid("vec4   main(float2 p) { return p.xyxy; }");
    test_valid("half4  main(vec2   p) { return p.xyxy; }");
    test_valid("vec4   main(vec2   p) { return p.xyxy; }");

    // The 'half4 main(float2, half4|float4)' signature is disallowed on both public and private
    // runtime effects.
    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::AllowPrivateAccess(&options);
    test_invalid("half4  main(float2 p, half4  c) { return c; }", "'main' parameter");
    test_invalid("half4  main(float2 p, half4  c) { return c; }", "'main' parameter", options);

    test_invalid("half4  main(float2 p, float4 c) { return c; }", "'main' parameter");
    test_invalid("half4  main(float2 p, float4 c) { return c; }", "'main' parameter", options);

    test_invalid("half4  main(float2 p, vec4   c) { return c; }", "'main' parameter");
    test_invalid("half4  main(float2 p, vec4   c) { return c; }", "'main' parameter", options);

    test_invalid("float4 main(float2 p, half4  c) { return c; }", "'main' parameter");
    test_invalid("float4 main(float2 p, half4  c) { return c; }", "'main' parameter", options);

    test_invalid("vec4   main(float2 p, half4  c) { return c; }", "'main' parameter");
    test_invalid("vec4   main(float2 p, half4  c) { return c; }", "'main' parameter", options);

    test_invalid("vec4   main(vec2   p, vec4   c) { return c; }", "'main' parameter");
    test_invalid("vec4   main(vec2   p, vec4   c) { return c; }", "'main' parameter", options);

    // Invalid return types
    test_invalid("void  main(float2 p) {}",                "'main' must return");
    test_invalid("half3 main(float2 p) { return p.xy1; }", "'main' must return");

    // Invalid argument types (some are valid as color filters, but not shaders)
    test_invalid("half4 main() { return half4(1); }", "'main' parameter");
    test_invalid("half4 main(half4 c) { return c; }", "'main' parameter");

    // sk_FragCoord should be available, but only if we've enabled it via Options
    test_invalid("half4 main(float2 p) { return sk_FragCoord.xy01; }",
                 "unknown identifier 'sk_FragCoord'");

    test_valid("half4 main(float2 p) { return sk_FragCoord.xy01; }", options);

    // Sampling a child shader requires that we pass explicit coords
    test_valid("uniform shader child;"
               "half4 main(float2 p) { return child.eval(p); }");

    // Sampling a colorFilter requires a color
    test_valid("uniform colorFilter child;"
               "half4 main(float2 p) { return child.eval(half4(1)); }");

    // Sampling a blender requires two colors
    test_valid("uniform blender child;"
               "half4 main(float2 p) { return child.eval(half4(0.5), half4(0.6)); }");
}

using PreTestFn = std::function<void(SkCanvas*, SkPaint*)>;

void paint_canvas(SkCanvas* canvas, SkPaint* paint, const PreTestFn& preTestCallback) {
    canvas->save();
    if (preTestCallback) {
        preTestCallback(canvas, paint);
    }
    canvas->drawPaint(*paint);
    canvas->restore();
}

static bool read_pixels(SkSurface* surface,
                        GrColor* pixels) {
    SkImageInfo info = surface->imageInfo();
    SkPixmap dest{info, pixels, info.minRowBytes()};
    return surface->readPixels(dest, /*srcX=*/0, /*srcY=*/0);
}

static void verify_2x2_surface_results(skiatest::Reporter* r,
                                       const SkRuntimeEffect* effect,
                                       SkSurface* surface,
                                       std::array<GrColor, 4> expected) {
    std::array<GrColor, 4> actual;
    SkImageInfo info = surface->imageInfo();
    if (!read_pixels(surface, actual.data())) {
        REPORT_FAILURE(r, "readPixels", SkString("readPixels failed"));
        return;
    }

    if (actual != expected) {
        REPORT_FAILURE(r, "Runtime effect didn't match expectations",
                       SkStringPrintf("\n"
                                      "Expected: [ %08x %08x %08x %08x ]\n"
                                      "Got     : [ %08x %08x %08x %08x ]\n"
                                      "SkSL:\n%s\n",
                                      expected[0], expected[1], expected[2], expected[3],
                                      actual[0],   actual[1],   actual[2],   actual[3],
                                      effect->source().c_str()));
    }
}

static sk_sp<SkSurface> make_surface(GrRecordingContext* grContext,
                                     const GraphiteInfo* graphite,
                                     SkISize size) {
    const SkImageInfo info = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface;
    if (graphite) {
#if defined(SK_GRAPHITE)
        surface = SkSurfaces::RenderTarget(graphite->recorder, info);
#endif
    } else if (grContext) {
        surface = SkSurfaces::RenderTarget(grContext, skgpu::Budgeted::kNo, info);
    } else {
        surface = SkSurfaces::Raster(info);
    }
    SkASSERT(surface);
    return surface;
}

class TestEffect {
public:
    TestEffect(skiatest::Reporter* r,
               GrRecordingContext* grContext,
               const GraphiteInfo* graphite,
               SkISize size = {2, 2})
            : fReporter(r), fGrContext(grContext), fGraphite(graphite), fSize(size) {
        fSurface = make_surface(fGrContext, fGraphite, fSize);
    }

    void build(const char* src) {
        SkRuntimeEffect::Options options;
        SkRuntimeEffectPriv::AllowPrivateAccess(&options);
        auto [effect, errorText] = SkRuntimeEffect::MakeForShader(SkString(src), options);
        if (!effect) {
            ERRORF(fReporter, "Effect didn't compile: %s", errorText.c_str());
            return;
        }
        fBuilder.init(std::move(effect));
    }

    SkRuntimeShaderBuilder::BuilderUniform uniform(const char* name) {
        return fBuilder->uniform(name);
    }

    SkRuntimeShaderBuilder::BuilderChild child(const char* name) {
        return fBuilder->child(name);
    }

    void test(std::array<GrColor, 4> expected, PreTestFn preTestCallback = nullptr) {
        auto shader = fBuilder->makeShader();
        if (!shader) {
            ERRORF(fReporter, "Effect didn't produce a shader");
            return;
        }

        SkCanvas* canvas = fSurface->getCanvas();

        // We shouldn't need to clear the canvas, because we are about to paint over the whole thing
        // with a `source` blend mode. However, there are a few devices where the background can
        // leak through when we paint with MSAA on. (This seems to be a driver/hardware bug.)
        // Graphite, at present, uses MSAA to do `drawPaint`. To avoid flakiness in this test on
        // those devices, we explicitly clear the canvas here. (skia:13761)
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setShader(std::move(shader));
        paint.setBlendMode(SkBlendMode::kSrc);

        paint_canvas(canvas, &paint, preTestCallback);

        verify_2x2_surface_results(fReporter, fBuilder->effect(), fSurface.get(), expected);
    }

    std::string trace(const SkIPoint& traceCoord) {
        sk_sp<SkShader> shader = fBuilder->makeShader();
        if (!shader) {
            ERRORF(fReporter, "Effect didn't produce a shader");
            return {};
        }

        auto [debugShader, debugTrace] = SkRuntimeEffect::MakeTraced(std::move(shader), traceCoord);

        SkCanvas* canvas = fSurface->getCanvas();
        SkPaint paint;
        paint.setShader(std::move(debugShader));
        paint.setBlendMode(SkBlendMode::kSrc);

        paint_canvas(canvas, &paint, /*preTestCallback=*/nullptr);

        SkDynamicMemoryWStream wstream;
        debugTrace->dump(&wstream);
        sk_sp<SkData> streamData = wstream.detachAsData();
        return std::string(static_cast<const char*>(streamData->data()), streamData->size());
    }

    void test(GrColor expected, PreTestFn preTestCallback = nullptr) {
        this->test({expected, expected, expected, expected}, preTestCallback);
    }

private:
    skiatest::Reporter*             fReporter;
    sk_sp<SkSurface>                fSurface;
    GrRecordingContext*             fGrContext;
    const GraphiteInfo*             fGraphite;
    SkISize                         fSize;
    SkTLazy<SkRuntimeShaderBuilder> fBuilder;
};

class TestBlend {
public:
    TestBlend(skiatest::Reporter* r, GrRecordingContext* grContext, const GraphiteInfo* graphite)
            : fReporter(r), fGrContext(grContext), fGraphite(graphite) {
        fSurface = make_surface(fGrContext, fGraphite, /*size=*/{2, 2});
    }

    void build(const char* src) {
        auto [effect, errorText] = SkRuntimeEffect::MakeForBlender(SkString(src));
        if (!effect) {
            ERRORF(fReporter, "Effect didn't compile: %s", errorText.c_str());
            return;
        }
        fBuilder.init(std::move(effect));
    }

    SkSurface* surface() {
        return fSurface.get();
    }

    SkRuntimeBlendBuilder::BuilderUniform uniform(const char* name) {
        return fBuilder->uniform(name);
    }

    SkRuntimeBlendBuilder::BuilderChild child(const char* name) {
        return fBuilder->child(name);
    }

    void test(std::array<GrColor, 4> expected, PreTestFn preTestCallback = nullptr) {
        auto blender = fBuilder->makeBlender();
        if (!blender) {
            ERRORF(fReporter, "Effect didn't produce a blender");
            return;
        }

        SkCanvas* canvas = fSurface->getCanvas();
        SkPaint paint;
        paint.setBlender(std::move(blender));
        paint.setColor(SK_ColorGRAY);

        paint_canvas(canvas, &paint, preTestCallback);

        verify_2x2_surface_results(fReporter, fBuilder->effect(), fSurface.get(), expected);
    }

    void test(GrColor expected, PreTestFn preTestCallback = nullptr) {
        this->test({expected, expected, expected, expected}, preTestCallback);
    }

private:
    skiatest::Reporter*            fReporter;
    sk_sp<SkSurface>               fSurface;
    GrRecordingContext*            fGrContext;
    const GraphiteInfo*            fGraphite;
    SkTLazy<SkRuntimeBlendBuilder> fBuilder;
};

// Produces a shader which will paint these opaque colors in a 2x2 rectangle:
// [  Red, Green ]
// [ Blue, White ]
static sk_sp<SkShader> make_RGBW_shader() {
    static constexpr SkColor colors[] = {SK_ColorWHITE, SK_ColorWHITE,
                                         SK_ColorBLUE, SK_ColorBLUE,
                                         SK_ColorRED, SK_ColorRED,
                                         SK_ColorGREEN, SK_ColorGREEN};
    static constexpr SkScalar   pos[] = { 0, .25f, .25f, .50f, .50f, .75, .75, 1 };
    static_assert(std::size(colors) == std::size(pos), "size mismatch");
    return SkGradientShader::MakeSweep(1, 1, colors, pos, std::size(colors));
}

static void test_RuntimeEffect_Shaders(skiatest::Reporter* r,
                                       GrRecordingContext* grContext,
                                       const GraphiteInfo* graphite) {
    TestEffect effect(r, grContext, graphite);
    using float4 = std::array<float, 4>;
    using int4 = std::array<int, 4>;

    // Local coords
    effect.build("half4 main(float2 p) { return half4(half2(p - 0.5), 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    effect.build("uniform float4 gColor; half4 main(float2 p) { return half4(gColor); }");
    effect.uniform("gColor") = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
    effect.test(0x7F0000FF);  // Tests that we don't clamp to valid premul

    // Same, with integer uniforms
    effect.build("uniform int4 gColor; half4 main(float2 p) { return half4(gColor) / 255.0; }");
    effect.uniform("gColor") = int4{ 0x00, 0x40, 0xBF, 0xFF };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = int4{ 0xFF, 0x00, 0x00, 0x7F };
    effect.test(0x7F0000FF);  // Tests that we don't clamp to valid premul

    // Test sk_FragCoord (device coords). Rotate the canvas to be sure we're seeing device coords.
    // Since the surface is 2x2, we should see (0,0), (1,0), (0,1), (1,1). Multiply by 0.498 to
    // make sure we're not saturating unexpectedly.
    effect.build(
            "half4 main(float2 p) { return half4(0.498 * (half2(sk_FragCoord.xy) - 0.5), 0, 1); }");
    effect.test({0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F},
                [](SkCanvas* canvas, SkPaint*) { canvas->rotate(45.0f); });

    // Runtime effects should use relaxed precision rules by default
    effect.build("half4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});

    // ... and support *returning* float4 (aka vec4), not just half4
    effect.build("float4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});
    effect.build("vec4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});

    // Mutating coords should work. (skbug.com/10918)
    effect.build("vec4 main(vec2 p) { p -= 0.5; return vec4(p, 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});
    effect.build("void moveCoords(inout vec2 p) { p -= 0.5; }"
                 "vec4 main(vec2 p) { moveCoords(p); return vec4(p, 0, 1); }");
    effect.test({0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF});

    //
    // Sampling children
    //

    // Sampling a null shader should return transparent black
    if (!graphite) {
        // TODO: Graphite does not yet pass this test.
        effect.build("uniform shader child;"
                     "half4 main(float2 p) { return child.eval(p); }");
        effect.child("child") = nullptr;
        effect.test(0x00000000,
                    [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });
    }

    // Sampling a null color-filter should return the passed-in color
    effect.build("uniform colorFilter child;"
                 "half4 main(float2 p) { return child.eval(half4(1, 1, 0, 1)); }");
    effect.child("child") = nullptr;
    effect.test(0xFF00FFFF);

    // Sampling a null blender should return blend_src_over(src, dest).
    effect.build("uniform blender child;"
                 "half4 main(float2 p) {"
                 "    float4 src = float4(p - 0.5, 0, 1) * 0.498;"
                 "    return child.eval(src, half4(0, 0, 0, 1));"
                 "}");
    effect.child("child") = nullptr;
    effect.test({0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F});

    // Sampling a simple child at our coordinates
    sk_sp<SkShader> rgbwShader = make_RGBW_shader();

    effect.build("uniform shader child;"
                 "half4 main(float2 p) { return child.eval(p); }");
    effect.child("child") = rgbwShader;
    effect.test({0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF});

    // Sampling with explicit coordinates (reflecting about the diagonal)
    effect.build("uniform shader child;"
                 "half4 main(float2 p) { return child.eval(p.yx); }");
    effect.child("child") = rgbwShader;
    effect.test({0xFF0000FF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFFFF});

    // Bind an image shader, but don't use it - ensure that we don't assert or generate bad shaders.
    // (skbug.com/12429)
    effect.build("uniform shader child;"
                 "half4 main(float2 p) { return half4(0, 1, 0, 1); }");
    effect.child("child") = rgbwShader;
    effect.test(0xFF00FF00);

    //
    // Helper functions
    //

    // Test case for inlining in the pipeline-stage and fragment-shader passes (skbug.com/10526):
    effect.build("float2 helper(float2 x) { return x + 1; }"
                 "half4 main(float2 p) { float2 v = helper(p); return half4(half2(v), 0, 1); }");
    effect.test(0xFF00FFFF);
}

DEF_TEST(SkRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, /*grContext=*/nullptr, /*graphite=*/nullptr);
}

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectSimple_Graphite, r, context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    GraphiteInfo graphite = {context, recorder.get()};
    test_RuntimeEffect_Shaders(r, /*grContext=*/nullptr, &graphite);
}
#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectSimple_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    test_RuntimeEffect_Shaders(r, ctxInfo.directContext(), /*graphite=*/nullptr);
}

static void verify_draw_obeys_capabilities(skiatest::Reporter* r,
                                           const SkRuntimeEffect* effect,
                                           SkSurface* surface,
                                           const SkPaint& paint) {
    // We expect the draw to do something if-and-only-if expectSuccess is true:
    const bool expectSuccess = surface->capabilities()->skslVersion() >= SkSL::Version::k300;

    constexpr GrColor kGreen = 0xFF00FF00;
    constexpr GrColor kRed   = 0xFF0000FF;
    const GrColor kExpected = expectSuccess ? kGreen : kRed;

    surface->getCanvas()->clear(SK_ColorRED);
    surface->getCanvas()->drawPaint(paint);
    verify_2x2_surface_results(r, effect, surface, {kExpected, kExpected, kExpected, kExpected});
}

static void test_RuntimeEffectObeysCapabilities(skiatest::Reporter* r, SkSurface* surface) {
    // This test creates shaders and blenders that target `#version 300`. If a user validates an
    // effect like this against a particular device, and later draws that effect to a device with
    // insufficient capabilities -- we want to fail gracefully (drop the draw entirely).
    // If the capabilities indicate that the effect is supported, we expect it to work.
    //
    // We test two different scenarios here:
    // 1) An effect flagged as #version 300, but actually compatible with #version 100.
    // 2) An effect flagged as #version 300, and using features not available in ES2.
    //
    // We expect both cases to fail cleanly on ES2-only devices -- nothing should be drawn, and
    // there should be no asserts or driver shader-compilation errors.
    //
    // In all tests, we first clear the canvas to RED, then draw an effect that (if it renders)
    // will fill the canvas with GREEN. We check that the final colors match our expectations,
    // based on the device capabilities.

    // Effect that would actually work on CPU/ES2, but should still fail on those devices:
    {
        auto effect = SkRuntimeEffect::MakeForShader(SkString(R"(
            #version 300
            half4 main(float2 xy) { return half4(0, 1, 0, 1); }
        )")).effect;
        REPORTER_ASSERT(r, effect);
        SkPaint paint;
        paint.setShader(effect->makeShader(/*uniforms=*/nullptr, /*children=*/{}));
        REPORTER_ASSERT(r, paint.getShader());
        verify_draw_obeys_capabilities(r, effect.get(), surface, paint);
    }

    // Effect that won't work on CPU/ES2 at all, and should fail gracefully on those devices.
    // We choose to use bit-pun intrinsics because SkSL doesn't automatically inject an extension
    // to enable them (like it does for derivatives). We pass a non-literal value so that SkSL's
    // constant folding doesn't elide them entirely before the driver sees the shader.
    {
        auto effect = SkRuntimeEffect::MakeForShader(SkString(R"(
            #version 300
            half4 main(float2 xy) {
                half4 result = half4(0, 1, 0, 1);
                result.g = intBitsToFloat(floatBitsToInt(result.g));
                return result;
            }
        )")).effect;
        REPORTER_ASSERT(r, effect);
        SkPaint paint;
        paint.setShader(effect->makeShader(/*uniforms=*/nullptr, /*children=*/{}));
        REPORTER_ASSERT(r, paint.getShader());
        verify_draw_obeys_capabilities(r, effect.get(), surface, paint);
    }

    //
    // As above, but with a blender
    //

    {
        auto effect = SkRuntimeEffect::MakeForBlender(SkString(R"(
            #version 300
            half4 main(half4 src, half4 dst) { return half4(0, 1, 0, 1); }
        )")).effect;
        REPORTER_ASSERT(r, effect);
        SkPaint paint;
        paint.setBlender(effect->makeBlender(/*uniforms=*/nullptr, /*children=*/{}));
        REPORTER_ASSERT(r, paint.getBlender());
        verify_draw_obeys_capabilities(r, effect.get(), surface, paint);
    }

    {
        auto effect = SkRuntimeEffect::MakeForBlender(SkString(R"(
            #version 300
            half4 main(half4 src, half4 dst) {
                half4 result = half4(0, 1, 0, 1);
                result.g = intBitsToFloat(floatBitsToInt(result.g));
                return result;
            }
        )")).effect;
        REPORTER_ASSERT(r, effect);
        SkPaint paint;
        paint.setBlender(effect->makeBlender(/*uniforms=*/nullptr, /*children=*/{}));
        REPORTER_ASSERT(r, paint.getBlender());
        verify_draw_obeys_capabilities(r, effect.get(), surface, paint);
    }
}

DEF_TEST(SkRuntimeEffectObeysCapabilities_CPU, r) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    REPORTER_ASSERT(r, surface);
    test_RuntimeEffectObeysCapabilities(r, surface.get());
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectObeysCapabilities_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_U) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(r, surface);
    test_RuntimeEffectObeysCapabilities(r, surface.get());
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeColorFilterReturningInvalidAlpha_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(r, surface);

    auto effect = SkRuntimeEffect::MakeForColorFilter(SkString(R"(
        half4 main(half4 color) { return half4(2); }
    )")).effect;
    REPORTER_ASSERT(r, effect);
    SkPaint paint;
    paint.setColorFilter(effect->makeColorFilter(/*uniforms=*/nullptr));
    REPORTER_ASSERT(r, paint.getColorFilter());
    surface->getCanvas()->drawPaint(paint);
}

DEF_TEST(SkRuntimeColorFilterLimitedToES2, r) {
    // Verify that SkSL requesting #version 300 can't be used to create a color-filter effect.
    // This restriction could be removed if we can find a way to implement filterColor4f for these
    // color filters.
    {
        auto effect = SkRuntimeEffect::MakeForColorFilter(SkString(R"(
            #version 300
            half4 main(half4 inColor) { return half4(1, 0, 0, 1); }
        )")).effect;
        REPORTER_ASSERT(r, !effect);
    }

    {
        auto effect = SkRuntimeEffect::MakeForColorFilter(SkString(R"(
            #version 300
            uniform int loops;
            half4 main(half4 inColor) {
                half4 result = half4(1, 0, 0, 1);
                for (int i = 0; i < loops; i++) {
                    result = result.argb;
                }
                return result;
            }
        )")).effect;
        REPORTER_ASSERT(r, !effect);
    }
}

DEF_TEST(SkRuntimeEffectTraceShader, r) {
    for (int imageSize : {2, 80}) {
        TestEffect effect(r, /*grContext=*/nullptr, /*graphite=*/nullptr,
                          SkISize{imageSize, imageSize});
        effect.build(R"(
            half4 main(float2 p) {
                float2 val = p - 0.5;
                return val.0y01;
            }
        )");
        int center = imageSize / 2;
        std::string dump = effect.trace({center, 1});
        static constexpr char kSkRPSlotDump[] =
R"($0 = p (float2 : slot 1/2, L0)
$1 = p (float2 : slot 2/2, L0)
$2 = [main].result (float4 : slot 1/4, L0)
$3 = [main].result (float4 : slot 2/4, L0)
$4 = [main].result (float4 : slot 3/4, L0)
$5 = [main].result (float4 : slot 4/4, L0)
$6 = val (float2 : slot 1/2, L0)
$7 = val (float2 : slot 2/2, L0)
F0 = half4 main(float2 p)
)";
        auto expectedTrace = SkSL::String::printf(R"(
enter half4 main(float2 p)
  p.x = %d.5
  p.y = 1.5
  scope +1
   line 3
   val.x = %d
   val.y = 1
   line 4
   [main].result.x = 0
   [main].result.y = 1
   [main].result.z = 0
   [main].result.w = 1
  scope -1
exit half4 main(float2 p)
)", center, center);
        REPORTER_ASSERT(
                r,
                skstd::starts_with(dump, kSkRPSlotDump) && skstd::ends_with(dump, expectedTrace),
                "Trace does not match expectation for %dx%d:\n%.*s\n",
                imageSize, imageSize, (int)dump.size(), dump.data());
    }
}

DEF_TEST(SkRuntimeEffectTracesAreUnoptimized, r) {
    TestEffect effect(r, /*grContext=*/nullptr, /*graphite=*/nullptr);

    effect.build(R"(
        int globalUnreferencedVar = 7;
        half inlinableFunction() {
            return 1;
        }
        half4 main(float2 p) {
            if (true) {
                int localUnreferencedVar = 7;
            }
            return inlinableFunction().xxxx;
        }
    )");
    std::string dump = effect.trace({1, 1});
    static constexpr char kSkRPSlotDump[] =
R"($0 = p (float2 : slot 1/2, L0)
$1 = p (float2 : slot 2/2, L0)
$2 = globalUnreferencedVar (int, L0)
$3 = [main].result (float4 : slot 1/4, L0)
$4 = [main].result (float4 : slot 2/4, L0)
$5 = [main].result (float4 : slot 3/4, L0)
$6 = [main].result (float4 : slot 4/4, L0)
$7 = localUnreferencedVar (int, L0)
$8 = [inlinableFunction].result (float, L0)
F0 = half4 main(float2 p)
F1 = half inlinableFunction()
)";
    static constexpr char kExpectedTrace[] = R"(
globalUnreferencedVar = 7
enter half4 main(float2 p)
  p.x = 1.5
  p.y = 1.5
  scope +1
   line 7
   scope +1
    line 8
    localUnreferencedVar = 7
   scope -1
   line 10
   enter half inlinableFunction()
     scope +1
      line 4
      [inlinableFunction].result = 1
     scope -1
   exit half inlinableFunction()
   [main].result.x = 1
   [main].result.y = 1
   [main].result.z = 1
   [main].result.w = 1
  scope -1
exit half4 main(float2 p)
)";
    REPORTER_ASSERT(
            r,
            skstd::starts_with(dump, kSkRPSlotDump) && skstd::ends_with(dump, kExpectedTrace),
            "Trace output does not match expectation:\n%.*s\n", (int)dump.size(), dump.data());
}

DEF_TEST(SkRuntimeEffectTraceCodeThatCannotBeUnoptimized, r) {
    TestEffect effect(r, /*grContext=*/nullptr, /*graphite=*/nullptr);

    effect.build(R"(
        half4 main(float2 p) {
            int variableThatGetsOptimizedAway = 7;
            if (true) {
                return half4(1);
            }
            // This (unreachable) path doesn't return a value.
            // Without optimization, SkSL thinks this code doesn't return a value on every path.
        }
    )");
    std::string dump = effect.trace({1, 1});
    static constexpr char kSkRPSlotDump[] =
R"($0 = p (float2 : slot 1/2, L0)
$1 = p (float2 : slot 2/2, L0)
$2 = [main].result (float4 : slot 1/4, L0)
$3 = [main].result (float4 : slot 2/4, L0)
$4 = [main].result (float4 : slot 3/4, L0)
$5 = [main].result (float4 : slot 4/4, L0)
F0 = half4 main(float2 p)
)";
    static constexpr char kExpectedTrace[] = R"(
enter half4 main(float2 p)
  p.x = 1.5
  p.y = 1.5
  scope +1
   scope +1
    line 5
    [main].result.x = 1
    [main].result.y = 1
    [main].result.z = 1
    [main].result.w = 1
   scope -1
  scope -1
exit half4 main(float2 p)
)";
    REPORTER_ASSERT(
            r,
            skstd::starts_with(dump, kSkRPSlotDump) && skstd::ends_with(dump, kExpectedTrace),
            "Trace output does not match expectation:\n%.*s\n", (int)dump.size(), dump.data());
}

static void test_RuntimeEffect_Blenders(skiatest::Reporter* r,
                                        GrRecordingContext* grContext,
                                        const GraphiteInfo* graphite) {
    TestBlend effect(r, grContext, graphite);

    using float2 = std::array<float, 2>;
    using float4 = std::array<float, 4>;
    using int4 = std::array<int, 4>;

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    effect.build("uniform float4 gColor; half4 main(half4 s, half4 d) { return half4(gColor); }");
    effect.uniform("gColor") = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
    effect.test(0x7F0000FF);  // We don't clamp here either

    // Same, with integer uniforms
    effect.build("uniform int4 gColor;"
                 "half4 main(half4 s, half4 d) { return half4(gColor) / 255.0; }");
    effect.uniform("gColor") = int4{ 0x00, 0x40, 0xBF, 0xFF };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = int4{ 0xFF, 0x00, 0x00, 0x7F };
    effect.test(0x7F0000FF);  // We don't clamp here either

    // Verify that mutating the source and destination colors is allowed
    effect.build("half4 main(half4 s, half4 d) { s += d; d += s; return half4(1); }");
    effect.test(0xFFFFFFFF);

    // Verify that we can write out the source color (ignoring the dest color)
    // This is equivalent to the kSrc blend mode.
    effect.build("half4 main(half4 s, half4 d) { return s; }");
    effect.test(0xFF888888);

    // Fill the destination with a variety of colors (using the RGBW shader)
    SkPaint rgbwPaint;
    rgbwPaint.setShader(make_RGBW_shader());
    rgbwPaint.setBlendMode(SkBlendMode::kSrc);
    effect.surface()->getCanvas()->drawPaint(rgbwPaint);

    // Verify that we can read back the dest color exactly as-is (ignoring the source color)
    // This is equivalent to the kDst blend mode.
    effect.build("half4 main(half4 s, half4 d) { return d; }");
    effect.test({0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF});

    // Verify that we can invert the destination color (including the alpha channel).
    // The expected outputs are the exact inverse of the previous test.
    effect.build("half4 main(half4 s, half4 d) { return half4(1) - d; }");
    effect.test({0x00FFFF00, 0x00FF00FF, 0x0000FFFF, 0x00000000});

    // Verify that color values are clamped to 0 and 1.
    effect.build("half4 main(half4 s, half4 d) { return half4(-1); }");
    effect.test(0x00000000);
    effect.build("half4 main(half4 s, half4 d) { return half4(2); }");
    effect.test(0xFFFFFFFF);

    //
    // Sampling children
    //

    // Sampling a null shader should return transparent black.
    effect.build("uniform shader child;"
                 "half4 main(half4 s, half4 d) { return child.eval(s.rg); }");
    effect.child("child") = nullptr;
    effect.test(0x00000000,
                [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });

    effect.build("uniform colorFilter child;"
                 "half4 main(half4 s, half4 d) { return child.eval(s); }");
    effect.child("child") = nullptr;
    effect.test(0xFF00FFFF,
                [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });

    // Sampling a null blender should do a src-over blend. Draw 50% black over RGBW to verify this.
    effect.surface()->getCanvas()->drawPaint(rgbwPaint);
    effect.build("uniform blender child;"
                 "half4 main(half4 s, half4 d) { return child.eval(s, d); }");
    effect.child("child") = nullptr;
    effect.test({0xFF000080, 0xFF008000, 0xFF800000, 0xFF808080},
                [](SkCanvas*, SkPaint* paint) { paint->setColor4f({0.0f, 0.0f, 0.0f, 0.497f}); });

    // Sampling a shader at various coordinates
    effect.build("uniform shader child;"
                 "uniform half2 pos;"
                 "half4 main(half4 s, half4 d) { return child.eval(pos); }");
    effect.child("child") = make_RGBW_shader();
    effect.uniform("pos") = float2{0.5, 0.5};
    effect.test(0xFF0000FF);

    effect.uniform("pos") = float2{1.5, 0.5};
    effect.test(0xFF00FF00);

    effect.uniform("pos") = float2{0.5, 1.5};
    effect.test(0xFFFF0000);

    effect.uniform("pos") = float2{1.5, 1.5};
    effect.test(0xFFFFFFFF);

    // Sampling a color filter
    effect.build("uniform colorFilter child;"
                 "half4 main(half4 s, half4 d) { return child.eval(half4(1)); }");
    effect.child("child") = SkColorFilters::Blend(0xFF012345, SkBlendMode::kSrc);
    effect.test(0xFF452301);

    // Sampling a built-in blender
    effect.surface()->getCanvas()->drawPaint(rgbwPaint);
    effect.build("uniform blender child;"
                 "half4 main(half4 s, half4 d) { return child.eval(s, d); }");
    effect.child("child") = SkBlender::Mode(SkBlendMode::kPlus);
    effect.test({0xFF4523FF, 0xFF45FF01, 0xFFFF2301, 0xFFFFFFFF},
                [](SkCanvas*, SkPaint* paint) { paint->setColor(0xFF012345); });

    // Sampling a runtime-effect blender
    effect.surface()->getCanvas()->drawPaint(rgbwPaint);
    effect.build("uniform blender child;"
                 "half4 main(half4 s, half4 d) { return child.eval(s, d); }");
    effect.child("child") = SkBlenders::Arithmetic(0, 1, 1, 0, /*enforcePremul=*/false);
    effect.test({0xFF4523FF, 0xFF45FF01, 0xFFFF2301, 0xFFFFFFFF},
                [](SkCanvas*, SkPaint* paint) { paint->setColor(0xFF012345); });
}

DEF_TEST(SkRuntimeEffect_Blender_CPU, r) {
    test_RuntimeEffect_Blenders(r, /*grContext=*/nullptr, /*graphite=*/nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffect_Blender_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    test_RuntimeEffect_Blenders(r, ctxInfo.directContext(), /*graphite=*/nullptr);
}

DEF_TEST(SkRuntimeShaderBuilderReuse, r) {
    const char* kSource = R"(
        uniform half x;
        half4 main(float2 p) { return half4(x); }
    )";

    sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForShader(SkString(kSource)).effect;
    REPORTER_ASSERT(r, effect);

    // Test passes if this sequence doesn't assert.  skbug.com/10667
    SkRuntimeShaderBuilder b(std::move(effect));
    b.uniform("x") = 0.0f;
    auto shader_0 = b.makeShader();

    b.uniform("x") = 1.0f;
    auto shader_1 = b.makeShader();
}

DEF_TEST(SkRuntimeBlendBuilderReuse, r) {
    const char* kSource = R"(
        uniform half x;
        half4 main(half4 s, half4 d) { return half4(x); }
    )";

    sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForBlender(SkString(kSource)).effect;
    REPORTER_ASSERT(r, effect);

    // We should be able to construct multiple SkBlenders in a row without asserting.
    SkRuntimeBlendBuilder b(std::move(effect));
    for (float x = 0.0f; x <= 2.0f; x += 2.0f) {
        b.uniform("x") = x;
        sk_sp<SkBlender> blender = b.makeBlender();
    }
}

DEF_TEST(SkRuntimeShaderBuilderSetUniforms, r) {
    const char* kSource = R"(
        uniform half x;
        uniform vec2 offset;
        half4 main(float2 p) { return half4(x); }
    )";

    sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForShader(SkString(kSource)).effect;
    REPORTER_ASSERT(r, effect);

    SkRuntimeShaderBuilder b(std::move(effect));

    // Test passes if this sequence doesn't assert.
    float x = 1.0f;
    REPORTER_ASSERT(r, b.uniform("x").set(&x, 1));

    // add extra value to ensure that set doesn't try to use sizeof(array)
    float origin[] = { 2.0f, 3.0f, 4.0f };
    REPORTER_ASSERT(r, b.uniform("offset").set<float>(origin, 2));

#ifndef SK_DEBUG
    REPORTER_ASSERT(r, !b.uniform("offset").set<float>(origin, 1));
    REPORTER_ASSERT(r, !b.uniform("offset").set<float>(origin, 3));
#endif

    auto shader = b.makeShader();
}

DEF_TEST(SkRuntimeEffectThreaded, r) {
    // This tests that we can safely use SkRuntimeEffect::MakeForShader from more than one thread,
    // and also that programs don't refer to shared structures owned by the compiler.
    // skbug.com/10589
    static constexpr char kSource[] = "half4 main(float2 p) { return sk_FragCoord.xyxy; }";

    std::thread threads[16];
    for (auto& thread : threads) {
        thread = std::thread([r]() {
            SkRuntimeEffect::Options options;
            SkRuntimeEffectPriv::AllowPrivateAccess(&options);
            auto [effect, error] = SkRuntimeEffect::MakeForShader(SkString(kSource), options);
            REPORTER_ASSERT(r, effect);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

DEF_TEST(SkRuntimeEffectAllowsPrivateAccess, r) {
    SkRuntimeEffect::Options defaultOptions;
    SkRuntimeEffect::Options optionsWithAccess;
    SkRuntimeEffectPriv::AllowPrivateAccess(&optionsWithAccess);

    // Confirm that shaders can only access $private_functions when private access is allowed.
    {
        static constexpr char kShader[] =
                "half4 main(float2 p) { return $hsl_to_rgb(p.xxx, p.y); }";
        SkRuntimeEffect::Result normal =
                SkRuntimeEffect::MakeForShader(SkString(kShader), defaultOptions);
        REPORTER_ASSERT(r, !normal.effect);
        SkRuntimeEffect::Result privileged =
                SkRuntimeEffect::MakeForShader(SkString(kShader), optionsWithAccess);
        REPORTER_ASSERT(r, privileged.effect, "%s", privileged.errorText.c_str());
    }

    // Confirm that color filters can only access $private_functions when private access is allowed.
    {
        static constexpr char kColorFilter[] =
                "half4 main(half4 c)  { return $hsl_to_rgb(c.rgb, c.a); }";
        SkRuntimeEffect::Result normal =
                SkRuntimeEffect::MakeForColorFilter(SkString(kColorFilter), defaultOptions);
        REPORTER_ASSERT(r, !normal.effect);
        SkRuntimeEffect::Result privileged =
                SkRuntimeEffect::MakeForColorFilter(SkString(kColorFilter), optionsWithAccess);
        REPORTER_ASSERT(r, privileged.effect, "%s", privileged.errorText.c_str());
    }

    // Confirm that blenders can only access $private_functions when private access is allowed.
    {
        static constexpr char kBlender[] =
                "half4 main(half4 s, half4 d) { return $hsl_to_rgb(s.rgb, d.a); }";
        SkRuntimeEffect::Result normal =
                SkRuntimeEffect::MakeForBlender(SkString(kBlender), defaultOptions);
        REPORTER_ASSERT(r, !normal.effect);
        SkRuntimeEffect::Result privileged =
                SkRuntimeEffect::MakeForBlender(SkString(kBlender), optionsWithAccess);
        REPORTER_ASSERT(r, privileged.effect, "%s", privileged.errorText.c_str());
    }
}

DEF_TEST(SkRuntimeColorFilterSingleColor, r) {
    // Test runtime colorfilters support filterColor4f().
    auto [effect, err] =
            SkRuntimeEffect::MakeForColorFilter(SkString{"half4 main(half4 c) { return c*c; }"});
    REPORTER_ASSERT(r, effect);
    REPORTER_ASSERT(r, err.isEmpty());

    sk_sp<SkColorFilter> cf = effect->makeColorFilter(SkData::MakeEmpty());
    REPORTER_ASSERT(r, cf);

    SkColor4f c = cf->filterColor4f({0.25, 0.5, 0.75, 1.0},
                                    sk_srgb_singleton(), sk_srgb_singleton());
    REPORTER_ASSERT(r, c.fR == 0.0625f);
    REPORTER_ASSERT(r, c.fG == 0.25f);
    REPORTER_ASSERT(r, c.fB == 0.5625f);
    REPORTER_ASSERT(r, c.fA == 1.0f);
}

static void test_RuntimeEffectStructNameReuse(skiatest::Reporter* r, GrRecordingContext* rContext) {
    // Test that two different runtime effects can reuse struct names in a single paint operation
    auto [childEffect, err] = SkRuntimeEffect::MakeForShader(SkString(
        "uniform shader paint;"
        "struct S { half4 rgba; };"
        "void process(inout S s) { s.rgba.rgb *= 0.5; }"
        "half4 main(float2 p) { S s; s.rgba = paint.eval(p); process(s); return s.rgba; }"
    ));
    REPORTER_ASSERT(r, childEffect, "%s\n", err.c_str());
    sk_sp<SkShader> sourceColor = SkShaders::Color({0.99608f, 0.50196f, 0.0f, 1.0f}, nullptr);
    const GrColor kExpected = 0xFF00407F;
    sk_sp<SkShader> child = childEffect->makeShader(/*uniforms=*/nullptr,
                                                    &sourceColor,
                                                    /*childCount=*/1);

    TestEffect effect(r, /*grContext=*/nullptr, /*graphite=*/nullptr);
    effect.build(
            "uniform shader child;"
            "struct S { float2 coord; };"
            "void process(inout S s) { s.coord = s.coord.yx; }"
            "half4 main(float2 p) { S s; s.coord = p; process(s); return child.eval(s.coord); "
            "}");
    effect.child("child") = child;
    effect.test(kExpected, [](SkCanvas*, SkPaint* paint) {});
}

DEF_TEST(SkRuntimeStructNameReuse, r) {
    test_RuntimeEffectStructNameReuse(r, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRuntimeStructNameReuse_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    test_RuntimeEffectStructNameReuse(r, ctxInfo.directContext());
}

DEF_TEST(SkRuntimeColorFilterFlags, r) {
    auto expectAlphaUnchanged = [&](const char* shader) {
        auto [effect, err] = SkRuntimeEffect::MakeForColorFilter(SkString{shader});
        REPORTER_ASSERT(r, effect && err.isEmpty(), "%s", shader);
        sk_sp<SkColorFilter> filter = effect->makeColorFilter(SkData::MakeEmpty());
        REPORTER_ASSERT(r, filter && filter->isAlphaUnchanged(), "%s", shader);
    };

    auto expectAlphaChanged = [&](const char* shader) {
        auto [effect, err] = SkRuntimeEffect::MakeForColorFilter(SkString{shader});
        REPORTER_ASSERT(r, effect && err.isEmpty(), "%s", shader);
        sk_sp<SkColorFilter> filter = effect->makeColorFilter(SkData::MakeEmpty());
        REPORTER_ASSERT(r, filter && !filter->isAlphaUnchanged(), "%s", shader);
    };

    // We expect these patterns to be detected as alpha-unchanged.
    expectAlphaUnchanged("half4 main(half4 color) { return color; }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.aaaa; }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.bgra; }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.rraa; }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.010a; }");
    expectAlphaUnchanged("half4 main(half4 color) { return half4(0, 0, 0, color.a); }");
    expectAlphaUnchanged("half4 main(half4 color) { return half4(half2(1), color.ba); }");
    expectAlphaUnchanged("half4 main(half4 color) { return half4(half2(1), half2(color.a)); }");
    expectAlphaUnchanged("half4 main(half4 color) { return half4(color.a); }");
    expectAlphaUnchanged("half4 main(half4 color) { return half4(float4(color.baba)); }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.r != color.g ? color :"
                                                                              " color.000a; }");
    expectAlphaUnchanged("half4 main(half4 color) { return color.a == color.r ? color.rrra : "
                                                          "color.g == color.b ? color.ggga : "
                                                                            "   color.bbba; }");
    // Modifying the input color invalidates the check.
    expectAlphaChanged("half4 main(half4 color) { color.a = 0; return color; }");

    // These swizzles don't end in alpha.
    expectAlphaChanged("half4 main(half4 color) { return color.argb; }");
    expectAlphaChanged("half4 main(half4 color) { return color.rrrr; }");

    // This compound constructor doesn't end in alpha.
    expectAlphaChanged("half4 main(half4 color) { return half4(1, 1, 1, color.r); }");

    // This splat constructor doesn't use alpha.
    expectAlphaChanged("half4 main(half4 color) { return half4(color.r); }");

    // These ternaries don't return alpha on both sides
    expectAlphaChanged("half4 main(half4 color) { return color.a > 0 ? half4(0) : color; }");
    expectAlphaChanged("half4 main(half4 color) { return color.g < 1 ? color.bgra : color.abgr; }");
    expectAlphaChanged("half4 main(half4 color) { return color.b > 0.5 ? half4(0) : half4(1); }");

    // Performing arithmetic on the input causes it to report as "alpha changed" even if the
    // arithmetic is a no-op; we aren't smart enough to see through it.
    expectAlphaChanged("half4 main(half4 color) { return color + half4(1,1,1,0); }");
    expectAlphaChanged("half4 main(half4 color) { return color + half4(0,0,0,4); }");

    // All exit paths are checked.
    expectAlphaChanged("half4 main(half4 color) { "
                       "    if (color.r > 0.5) { return color; }"
                       "    return half4(0);"
                       "}");
    expectAlphaChanged("half4 main(half4 color) { "
                       "    if (color.r > 0.5) { return half4(0); }"
                       "    return color;"
                       "}");
}

DEF_TEST(SkRuntimeShaderSampleCoords, r) {
    // This test verifies that we detect calls to sample where the coords are the same as those
    // passed to main. In those cases, it's safe to turn the "explicit" sampling into "passthrough"
    // sampling. This optimization is implemented very conservatively.
    //
    // It also checks that we correctly set the "referencesSampleCoords" bit on the runtime effect
    // FP, depending on how the coords parameter to main is used.

    auto test = [&](const char* src, bool expectExplicit, bool expectReferencesSampleCoords) {
        auto [effect, err] =
                SkRuntimeEffect::MakeForShader(SkStringPrintf("uniform shader child; %s", src));
        REPORTER_ASSERT(r, effect);

        auto child = GrFragmentProcessor::MakeColor({ 1, 1, 1, 1 });
        auto fp = GrSkSLFP::Make(effect.get(), "test_fp", /*inputFP=*/nullptr,
                                 GrSkSLFP::OptFlags::kNone, "child", std::move(child));
        REPORTER_ASSERT(r, fp);

        REPORTER_ASSERT(r, fp->childProcessor(0)->sampleUsage().isExplicit() == expectExplicit);
        REPORTER_ASSERT(r, fp->usesSampleCoords() == expectReferencesSampleCoords);
    };

    // Cases where our optimization is valid, and works:

    // Direct use of passed-in coords. Here, the only use of sample coords is for a sample call
    // converted to passthrough, so referenceSampleCoords is *false*, despite appearing in main.
    test("half4 main(float2 xy) { return child.eval(xy); }", false, false);
    // Sample with passed-in coords, read (but don't write) sample coords elsewhere
    test("half4 main(float2 xy) { return child.eval(xy) + sin(xy.x); }", false, true);

    // Cases where our optimization is not valid, and does not happen:

    // Sampling with values completely unrelated to passed-in coords
    test("half4 main(float2 xy) { return child.eval(float2(0, 0)); }", true, false);
    // Use of expression involving passed in coords
    test("half4 main(float2 xy) { return child.eval(xy * 0.5); }", true, true);
    // Use of coords after modification
    test("half4 main(float2 xy) { xy *= 2; return child.eval(xy); }", true, true);
    // Use of coords after modification via out-param call
    test("void adjust(inout float2 xy) { xy *= 2; }"
         "half4 main(float2 xy) { adjust(xy); return child.eval(xy); }", true, true);

    // There should (must) not be any false-positive cases. There are false-negatives.
    // In all of these cases, our optimization would be valid, but does not happen:

    // Direct use of passed-in coords, modified after use
    test("half4 main(float2 xy) { half4 c = child.eval(xy); xy *= 2; return c; }", true, true);
    // Passed-in coords copied to a temp variable
    test("half4 main(float2 xy) { float2 p = xy; return child.eval(p); }", true, true);
    // Use of coords passed to helper function
    test("half4 helper(float2 xy) { return child.eval(xy); }"
         "half4 main(float2 xy) { return helper(xy); }", true, true);
}

DEF_TEST(SkRuntimeShaderIsOpaque, r) {
    // This test verifies that we detect certain simple patterns in runtime shaders, and can deduce
    // (via code in SkSL::Analysis::ReturnsOpaqueColor) that the resulting shader is always opaque.
    // That logic is conservative, and the tests below reflect this.

    auto test = [&](const char* body, bool expectOpaque) {
        auto [effect, err] = SkRuntimeEffect::MakeForShader(SkStringPrintf(R"(
            uniform shader cOnes;
            uniform shader cZeros;
            uniform float4 uOnes;
            uniform float4 uZeros;
            half4 main(float2 xy) {
                %s
            })", body));
        REPORTER_ASSERT(r, effect);

        auto cOnes = SkShaders::Color(SK_ColorWHITE);
        auto cZeros = SkShaders::Color(SK_ColorTRANSPARENT);
        SkASSERT(cOnes->isOpaque());
        SkASSERT(!cZeros->isOpaque());

        SkRuntimeShaderBuilder builder(effect);
        builder.child("cOnes") = std::move(cOnes);
        builder.child("cZeros") = std::move(cZeros);
        builder.uniform("uOnes") = SkColors::kWhite;
        builder.uniform("uZeros") = SkColors::kTransparent;

        auto shader = builder.makeShader();
        REPORTER_ASSERT(r, shader->isOpaque() == expectOpaque);
    };

    // Cases where our optimization is valid, and works:

    // Returning opaque literals
    test("return half4(1);",          true);
    test("return half4(0, 1, 0, 1);", true);
    test("return half4(0, 0, 0, 1);", true);

    // Simple expressions involving uniforms
    test("return uZeros.rgb1;",          true);
    test("return uZeros.bgra.rgb1;",     true);
    test("return half4(uZeros.rgb, 1);", true);

    // Simple expressions involving child.eval
    test("return cZeros.eval(xy).rgb1;",          true);
    test("return cZeros.eval(xy).bgra.rgb1;",     true);
    test("return half4(cZeros.eval(xy).rgb, 1);", true);

    // Multiple returns
    test("if (xy.x < 100) { return uZeros.rgb1; } else { return cZeros.eval(xy).rgb1; }", true);

    // More expression cases:
    test("return (cZeros.eval(xy) * uZeros).rgb1;", true);
    test("return half4(1, 1, 1, 0.5 + 0.5);",       true);

    // Constant variable propagation
    test("const half4 kWhite = half4(1); return kWhite;", true);

    // Cases where our optimization is not valid, and does not happen:

    // Returning non-opaque literals
    test("return half4(0);",          false);
    test("return half4(1, 1, 1, 0);", false);

    // Returning non-opaque uniforms or children
    test("return uZeros;",          false);
    test("return cZeros.eval(xy);", false);

    // Multiple returns
    test("if (xy.x < 100) { return uZeros; } else { return cZeros.eval(xy).rgb1; }", false);
    test("if (xy.x < 100) { return uZeros.rgb1; } else { return cZeros.eval(xy); }", false);

    // There should (must) not be any false-positive cases. There are false-negatives.
    // In these cases, our optimization would be valid, but does not happen:

    // More complex expressions that can't be simplified
    test("return xy.x < 100 ? uZeros.rgb1 : cZeros.eval(xy).rgb1;", false);

    // Finally, there are cases that are conditional on the uniforms and children. These *could*
    // determine dynamically if the uniform and/or child being referenced is opaque, and use that
    // information. Today, we don't do this, so we pessimistically assume they're transparent:
    test("return uOnes;",          false);
    test("return cOnes.eval(xy);", false);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(GrSkSLFP_Specialized, r, ctxInfo, CtsEnforcement::kApiLevel_T) {
    struct FpAndKey {
        std::unique_ptr<GrFragmentProcessor> fp;
        TArray<uint32_t, true>             key;
    };

    // Constant color, but with an 'specialize' option that decides if the color is inserted in the
    // SkSL as a literal, or left as a uniform
    auto make_color_fp = [&](SkPMColor4f color, bool specialize) {
        static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            "uniform half4 color;"
            "half4 main(float2 xy) { return color; }"
        );
        FpAndKey result;
        result.fp = GrSkSLFP::Make(effect, "color_fp", /*inputFP=*/nullptr,
                                   GrSkSLFP::OptFlags::kNone,
                                   "color", GrSkSLFP::SpecializeIf(specialize, color));
        skgpu::KeyBuilder builder(&result.key);
        result.fp->addToKey(*ctxInfo.directContext()->priv().caps()->shaderCaps(), &builder);
        builder.flush();
        return result;
    };

    FpAndKey uRed   = make_color_fp({1, 0, 0, 1}, false),
             uGreen = make_color_fp({0, 1, 0, 1}, false),
             sRed   = make_color_fp({1, 0, 0, 1}, true),
             sGreen = make_color_fp({0, 1, 0, 1}, true);

    // uRed and uGreen should have the same key - they just have different uniforms
    SkASSERT(uRed.key == uGreen.key);
    // sRed and sGreen should have keys that are different from the uniform case, and each other
    SkASSERT(sRed.key != uRed.key);
    SkASSERT(sGreen.key != uRed.key);
    SkASSERT(sRed.key != sGreen.key);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrSkSLFP_UniformArray,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    // Make a fill-context to draw into.
    GrDirectContext* directContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    std::unique_ptr<skgpu::ganesh::SurfaceFillContext> testCtx =
            directContext->priv().makeSFC(info, /*label=*/{}, SkBackingFit::kExact);

    // Make an effect that takes a uniform array as input.
    static constexpr std::array<float, 4> kRed  {1.0f, 0.0f, 0.0f, 1.0f};
    static constexpr std::array<float, 4> kGreen{0.0f, 1.0f, 0.0f, 1.0f};
    static constexpr std::array<float, 4> kBlue {0.0f, 0.0f, 1.0f, 1.0f};
    static constexpr std::array<float, 4> kGray {0.499f, 0.499f, 0.499f, 1.0f};

    for (const auto& colorArray : {kRed, kGreen, kBlue, kGray}) {
        // Compile our runtime effect.
        static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            "uniform half color[4];"
            "half4 main(float2 xy) { return half4(color[0], color[1], color[2], color[3]); }"
        );
        // Render our shader into the fill-context with our various input colors.
        testCtx->fillWithFP(GrSkSLFP::Make(effect, "test_fp", /*inputFP=*/nullptr,
                                           GrSkSLFP::OptFlags::kNone,
                                           "color", SkSpan(colorArray)));
        // Read our color back and ensure it matches.
        GrColor actual;
        GrPixmap pixmap(info, &actual, sizeof(GrColor));
        if (!testCtx->readPixels(directContext, pixmap, /*srcPt=*/{0, 0})) {
            REPORT_FAILURE(r, "readPixels", SkString("readPixels failed"));
            break;
        }
        if (actual != GrColorPackRGBA(255 * colorArray[0], 255 * colorArray[1],
                                      255 * colorArray[2], 255 * colorArray[3])) {
            REPORT_FAILURE(r, "Uniform array didn't match expectations",
                           SkStringPrintf("\n"
                                          "Expected: [ %g %g %g %g ]\n"
                                          "Got     : [ %08x ]\n",
                                          colorArray[0], colorArray[1],
                                          colorArray[2], colorArray[3],
                                          actual));
            break;
        }
    }
}
