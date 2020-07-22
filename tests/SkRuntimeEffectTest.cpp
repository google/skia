/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTLazy.h"
#include "tests/Test.h"

#include <algorithm>

DEF_TEST(SkRuntimeEffectInvalid, r) {
    auto test = [r](const char* hdr, const char* body, const char* expected) {
        SkString src = SkStringPrintf("%s void main(float2 p, inout half4 color) { %s }",
                                      hdr, body);
        auto[effect, errorText] = SkRuntimeEffect::Make(src);
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r, errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected, errorText.c_str());
    };

    // Features that are only allowed in .fp files (key, in uniform, ctype, when, tracked).
    // Ensure that these fail, and the error messages contain the relevant keyword.
    test("layout(key) in bool Input;", "", "key");
    test("in uniform float Input;", "", "in uniform");
    test("layout(ctype=SkRect) float4 Input;", "", "ctype");
    test("in bool Flag; layout(when=Flag) uniform float Input;", "", "when");
    test("layout(tracked) uniform float Input;", "", "tracked");

    // Runtime SkSL supports a limited set of uniform types. No samplers, for example:
    test("uniform sampler2D s;", "", "sampler2D");

    // 'in' variables can't be arrays
    test("in int Input[2];", "", "array");

    // Type specific restrictions:

    // 'bool', 'int' can't be 'uniform'
    test("uniform bool Input;", "", "'uniform'");
    test("uniform int Input;", "", "'uniform'");

    // vector and matrix types can't be 'in'
    test("in float2 Input;", "", "'in'");
    test("in half3x3 Input;", "", "'in'");

    // 'marker' is only permitted on 'uniform' variables
    test("layout(marker=local_to_world) in float4x4 localToWorld;", "", "'uniform'");
    // 'marker' is only permitted on float4x4 variables
    test("layout(marker=local_to_world) uniform float3x3 localToWorld;", "", "float4x4");

    test("half missing();", "color.r = missing();", "undefined function");

    // Shouldn't be possible to create an SkRuntimeEffect without "main"
    test("//", "", "main");
}

class TestEffect {
public:
    TestEffect(skiatest::Reporter* r, sk_sp<SkSurface> surface)
            : fReporter(r), fSurface(std::move(surface)) {}

    void build(const char* header, const char* body) {
        SkString src = SkStringPrintf("%s void main(float2 p, inout half4 color) { %s }",
                                      header, body);
        auto[effect, errorText] = SkRuntimeEffect::Make(src);
        if (!effect) {
            REPORT_FAILURE(fReporter, "effect",
                           SkStringPrintf("Effect didn't compile: %s", errorText.c_str()));
            return;
        }
        fBuilder.init(std::move(effect));
    }

    SkRuntimeShaderBuilder::BuilderInput operator[](const char* name) {
        return fBuilder->input(name);
    }

    using PreTestFn = std::function<void(SkPaint*, SkCanvas*)>;

    void test(uint32_t TL, uint32_t TR, uint32_t BL, uint32_t BR,
              PreTestFn preTestCallback = nullptr) {
        auto shader = fBuilder->makeShader(nullptr, false);
        if (!shader) {
            REPORT_FAILURE(fReporter, "shader", SkString("Effect didn't produce a shader"));
            return;
        }

        SkPaint paint;
        paint.setShader(std::move(shader));
        paint.setBlendMode(SkBlendMode::kSrc);

        if (preTestCallback) {
            preTestCallback(&paint, fSurface->getCanvas());
        }

        fSurface->getCanvas()->drawPaint(paint);

        uint32_t actual[4];
        SkImageInfo info = fSurface->imageInfo();
        if (!fSurface->readPixels(info, actual, info.minRowBytes(), 0, 0)) {
            REPORT_FAILURE(fReporter, "readPixels", SkString("readPixels failed"));
            return;
        }

        uint32_t expected[4] = {TL, TR, BL, BR};
        if (memcmp(actual, expected, sizeof(actual)) != 0) {
            REPORT_FAILURE(fReporter, "Runtime effect didn't match expectations",
                           SkStringPrintf("\n"
                                          "Expected: [ %08x %08x %08x %08x ]\n"
                                          "Got     : [ %08x %08x %08x %08x ]\n"
                                          "SkSL:\n%s\n",
                                          TL, TR, BL, BR, actual[0], actual[1], actual[2],
                                          actual[3], fBuilder->fEffect->source().c_str()));
        }
    }

    void test(uint32_t expected, PreTestFn preTestCallback = nullptr) {
        this->test(expected, expected, expected, expected, preTestCallback);
    }

private:
    skiatest::Reporter*             fReporter;
    sk_sp<SkSurface>                fSurface;
    SkTLazy<SkRuntimeShaderBuilder> fBuilder;
};

static void test_RuntimeEffect_Shaders(skiatest::Reporter* r, GrContext* context) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = context ? SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info)
                                       : SkSurface::MakeRaster(info);
    REPORTER_ASSERT(r, surface);
    TestEffect e(r, surface);

    using float4 = std::array<float, 4>;

    // Local coords
    e.build("", "color = half4(half2(p - 0.5), 0, 1);");
    e.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    e.build("uniform float4 gColor;",
            "color = half4(gColor);");
    e["gColor"] = float4{ 0.0f, 0.25f, 0.75f, 1.0f }; e.test(0xFFBF4000);
    e["gColor"] = float4{ 0.75f, 0.25f, 0.0f, 1.0f }; e.test(0xFF0040BF);

    // Indexing a uniform array with an 'in' integer
    e.build("in int flag; uniform half4 gColors[2];",
            "color = gColors[flag];");
    e["gColors"] = std::array<float4, 2>{float4{1.0f, 0.0f, 0.0f, 0.498f},
                                         float4{0.0f, 1.0f, 0.0f, 1.0f  }};
    e["flag"] = 0; e.test(0x7F00007F);  // Tests that we clamp to valid premul
    e["flag"] = 1; e.test(0xFF00FF00);

    // 'in' half (functionally a uniform, but handled very differently internally)
    e.build("in half c;",
            "color = half4(c, c, c, 1);");
    e["c"] = 0.498f; e.test(0xFF7F7F7F);

    // Test sk_FragCoord (device coords). Rotate the canvas to be sure we're seeing device coords.
    // Since the surface is 2x2, we should see (0,0), (1,0), (0,1), (1,1). Multiply by 0.498 to
    // make sure we're not saturating unexpectedly.
    e.build("", "color = half4(0.498 * (half2(sk_FragCoord.xy) - 0.5), 0, 1);");
    e.test(0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F,
           [](SkPaint*, SkCanvas* canvas) { canvas->rotate(45.0f); });
}

DEF_TEST(SkRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, nullptr);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectSimple_GPU, r, ctxInfo) {
    test_RuntimeEffect_Shaders(r, ctxInfo.directContext());
}
