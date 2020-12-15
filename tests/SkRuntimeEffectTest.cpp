/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "tests/Test.h"

#include <algorithm>
#include <thread>

DEF_TEST(SkRuntimeEffectInvalid, r) {
    auto test = [r](const char* src, const char* expected) {
        auto[effect, errorText] = SkRuntimeEffect::Make(SkString(src));
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r, errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected, errorText.c_str());
    };

#define EMPTY_MAIN "half4 main() { return half4(0); }"

    // Features that are only allowed in .fp files (key, in uniform, ctype, when, tracked).
    // Ensure that these fail, and the error messages contain the relevant keyword.
    test("layout(key) in bool Input;"                           EMPTY_MAIN, "key");
    test("in uniform float Input;"                              EMPTY_MAIN, "in uniform");
    test("layout(ctype=SkRect) float4 Input;"                   EMPTY_MAIN, "ctype");
    test("in bool Flag; layout(when=Flag) uniform float Input;" EMPTY_MAIN, "when");
    test("layout(tracked) uniform float Input;"                 EMPTY_MAIN, "tracked");

    // GLSL types like sampler2D and texture2D are not allowed anywhere:
    test("uniform sampler2D s;" EMPTY_MAIN, "no type named 'sampler2D'");
    test("uniform texture2D s;" EMPTY_MAIN, "no type named 'texture2D'");

    // Runtime SkSL supports a limited set of uniform types. No bool, or int, for example:
    test("uniform bool b;" EMPTY_MAIN, "uniform");
    test("uniform int i;"  EMPTY_MAIN, "uniform");

    // 'in' variables aren't allowed at all:
    test("in bool b;"    EMPTY_MAIN, "'in'");
    test("in float f;"   EMPTY_MAIN, "'in'");
    test("in float2 v;"  EMPTY_MAIN, "'in'");
    test("in half3x3 m;" EMPTY_MAIN, "'in'");

    // 'marker' is only permitted on float4x4 uniforms
    test("layout(marker=local_to_world) uniform float3x3 localToWorld;" EMPTY_MAIN, "float4x4");

    test("half4 missing(); half4 main() { return missing(); }", "undefined function");

    // No use of 'discard' is permitted
    test("half4 main() { discard; }", "discard");

    // Shouldn't be possible to create an SkRuntimeEffect without "main"
    test("", "main");

    // Various places that shaders (fragmentProcessors) should not be allowed
    test("half4 main() { shader child; return sample(child); }",
         "must be global");
    test("uniform shader child; half4 helper(shader fp) { return sample(fp); }"
         "half4 main() { return helper(child); }",
         "parameter");
    test("uniform shader child; shader get_child() { return child; }"
         "half4 main() { return sample(get_child()); }",
         "return");
    test("uniform shader child;"
         "half4 main() { return sample(shader(child)); }",
         "construct");
    test("uniform shader child1; uniform shader child2;"
         "half4 main(float2 p) { return sample(p.x > 10 ? child1 : child2); }",
         "expression");

    // sk_Caps is an internal system. It should not be visible to runtime effects
    test("half4 main() { return sk_Caps.integerSupport ? half4(1) : half4(0); }",
         "unknown identifier 'sk_Caps'");

    // Errors that aren't caught until later in the compilation process (during optimize())
    test("half4 main() { return half4(1); return half4(0); }", "unreachable");
    test("half4 badFunc() {}"
         "half4 main() { return badFunc(); }",
         "without returning");
}

DEF_TEST(SkRuntimeEffectInvalidColorFilters, r) {
    auto test = [r](const char* sksl) {
        auto [effect, errorText] = SkRuntimeEffect::Make(SkString(sksl));
        REPORTER_ASSERT(r, effect);

        sk_sp<SkData> uniforms = SkData::MakeUninitialized(effect->uniformSize());

        REPORTER_ASSERT(r, effect->makeShader(uniforms, nullptr, 0, nullptr, false));
        REPORTER_ASSERT(r, !effect->makeColorFilter(uniforms));
    };

    // Runtime effects that use sample coords or sk_FragCoord are valid shaders,
    // but not valid color filters
    test("half4 main(float2 p) { return half2(p).xy01; }");
    test("half4 main(float2 p) { return half2(sk_FragCoord.xy).xy01; }");

    // We also can't use layout(marker), which would give the runtime color filter CTM information
    test("layout(marker=ctm) uniform float4x4 ctm;"
         "half4 main(float2 p) { return half4(half(ctm[0][0]), 0, 0, 1); }");
}

class TestEffect {
public:
    TestEffect(skiatest::Reporter* r, sk_sp<SkSurface> surface)
            : fReporter(r), fSurface(std::move(surface)) {}

    void build(const char* src) {
        auto[effect, errorText] = SkRuntimeEffect::Make(SkString(src));
        if (!effect) {
            REPORT_FAILURE(fReporter, "effect",
                           SkStringPrintf("Effect didn't compile: %s", errorText.c_str()));
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

    using PreTestFn = std::function<void(SkCanvas*, SkPaint*)>;

    void test(GrColor TL, GrColor TR, GrColor BL, GrColor BR,
              PreTestFn preTestCallback = nullptr) {
        auto shader = fBuilder->makeShader(nullptr, false);
        if (!shader) {
            REPORT_FAILURE(fReporter, "shader", SkString("Effect didn't produce a shader"));
            return;
        }

        SkCanvas* canvas = fSurface->getCanvas();
        SkPaint paint;
        paint.setShader(std::move(shader));
        paint.setBlendMode(SkBlendMode::kSrc);

        canvas->save();
        if (preTestCallback) {
            preTestCallback(canvas, &paint);
        }
        canvas->drawPaint(paint);
        canvas->restore();

        GrColor actual[4];
        SkImageInfo info = fSurface->imageInfo();
        if (!fSurface->readPixels(info, actual, info.minRowBytes(), 0, 0)) {
            REPORT_FAILURE(fReporter, "readPixels", SkString("readPixels failed"));
            return;
        }

        GrColor expected[4] = {TL, TR, BL, BR};
        if (0 != memcmp(actual, expected, sizeof(actual))) {
            REPORT_FAILURE(fReporter, "Runtime effect didn't match expectations",
                           SkStringPrintf("\n"
                                          "Expected: [ %08x %08x %08x %08x ]\n"
                                          "Got     : [ %08x %08x %08x %08x ]\n"
                                          "SkSL:\n%s\n",
                                          TL, TR, BL, BR, actual[0], actual[1], actual[2],
                                          actual[3], fBuilder->effect()->source().c_str()));
        }
    }

    void test(GrColor expected, PreTestFn preTestCallback = nullptr) {
        this->test(expected, expected, expected, expected, preTestCallback);
    }

private:
    skiatest::Reporter*             fReporter;
    sk_sp<SkSurface>                fSurface;
    SkTLazy<SkRuntimeShaderBuilder> fBuilder;
};

// Produces a 2x2 bitmap shader, with opaque colors:
// [  Red, Green ]
// [ Blue, White ]
static sk_sp<SkShader> make_RGBW_shader() {
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkIRect topLeft = SkIRect::MakeWH(1, 1);
    bmp.pixmap().erase(SK_ColorRED,   topLeft);
    bmp.pixmap().erase(SK_ColorGREEN, topLeft.makeOffset(1, 0));
    bmp.pixmap().erase(SK_ColorBLUE,  topLeft.makeOffset(0, 1));
    bmp.pixmap().erase(SK_ColorWHITE, topLeft.makeOffset(1, 1));
    return bmp.makeShader(SkSamplingOptions());
}

static void test_RuntimeEffect_Shaders(skiatest::Reporter* r, GrRecordingContext* rContext) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = rContext
                                    ? SkSurface::MakeRenderTarget(rContext, SkBudgeted::kNo, info)
                                    : SkSurface::MakeRaster(info);
    REPORTER_ASSERT(r, surface);
    TestEffect effect(r, surface);

    using float4 = std::array<float, 4>;

    // Local coords
    effect.build("half4 main(float2 p) { return half4(half2(p - 0.5), 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    effect.build("uniform float4 gColor; half4 main() { return half4(gColor); }");
    effect.uniform("gColor") = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
    effect.test(0x7F00007F);  // Tests that we clamp to valid premul

    // Test sk_FragCoord (device coords). Rotate the canvas to be sure we're seeing device coords.
    // Since the surface is 2x2, we should see (0,0), (1,0), (0,1), (1,1). Multiply by 0.498 to
    // make sure we're not saturating unexpectedly.
    effect.build("half4 main() { return half4(0.498 * (half2(sk_FragCoord.xy) - 0.5), 0, 1); }");
    effect.test(0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F,
                [](SkCanvas* canvas, SkPaint*) { canvas->rotate(45.0f); });

    // Runtime effects should use relaxed precision rules by default
    effect.build("half4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // ... and support GLSL type names
    effect.build("half4 main(float2 p) { return vec4(p - 0.5, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // ... and support *returning* float4 (aka vec4), not just half4
    effect.build("float4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    effect.build("vec4 main(float2 p) { return float4(p - 0.5, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // Mutating coords should work. (skbug.com/10918)
    effect.build("vec4 main(vec2 p) { p -= 0.5; return vec4(p, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    effect.build("void moveCoords(inout vec2 p) { p -= 0.5; }"
                 "vec4 main(vec2 p) { moveCoords(p); return vec4(p, 0, 1); }");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    //
    // Sampling children
    //

    // Sampling a null child should return the paint color
    effect.build("uniform shader child;"
                 "half4 main() { return sample(child); }");
    effect.child("child") = nullptr;
    effect.test(0xFF00FFFF,
                [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });

    sk_sp<SkShader> rgbwShader = make_RGBW_shader();

    // Sampling a simple child at our coordinates (implicitly)
    effect.build("uniform shader child;"
                 "half4 main() { return sample(child); }");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF);

    // Sampling with explicit coordinates (reflecting about the diagonal)
    effect.build("uniform shader child;"
                 "half4 main(float2 p) { return sample(child, p.yx); }");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFFFF);

    // Sampling with a matrix (again, reflecting about the diagonal)
    effect.build("uniform shader child;"
                 "half4 main() { return sample(child, float3x3(0, 1, 0, 1, 0, 0, 0, 0, 1)); }");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFFFF);

    // Legacy behavior - shaders can be declared 'in' rather than 'uniform'
    effect.build("in shader child;"
                 "half4 main() { return sample(child); }");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF);

    //
    // Helper functions
    //

    // Test case for inlining in the pipeline-stage and fragment-shader passes (skbug.com/10526):
    effect.build("float2 helper(float2 x) { return x + 1; }"
                 "half4 main(float2 p) { float2 v = helper(p); return half4(half2(v), 0, 1); }");
    effect.test(0xFF00FFFF);
}

DEF_TEST(SkRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, nullptr);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectSimple_GPU, r, ctxInfo) {
    test_RuntimeEffect_Shaders(r, ctxInfo.directContext());
}

DEF_TEST(SkRuntimeShaderBuilderReuse, r) {
    const char* kSource = R"(
        uniform half x;
        half4 main() { return half4(x); }
    )";

    sk_sp<SkRuntimeEffect> effect = std::get<0>(SkRuntimeEffect::Make(SkString(kSource)));
    REPORTER_ASSERT(r, effect);

    // Test passes if this sequence doesn't assert.  skbug.com/10667
    SkRuntimeShaderBuilder b(std::move(effect));
    b.uniform("x") = 0.0f;
    auto shader_0 = b.makeShader(nullptr, false);

    b.uniform("x") = 1.0f;
    auto shader_1 = b.makeShader(nullptr, true);
}

DEF_TEST(SkRuntimeEffectThreaded, r) {
    // SkRuntimeEffect uses a single compiler instance, but it's mutex locked.
    // This tests that we can safely use it from more than one thread, and also
    // that programs don't refer to shared structures owned by the compiler.
    // skbug.com/10589
    static constexpr char kSource[] = "half4 main() { return sk_FragCoord.xyxy; }";

    std::thread threads[16];
    for (auto& thread : threads) {
        thread = std::thread([r]() {
            auto [effect, error] = SkRuntimeEffect::Make(SkString(kSource));
            REPORTER_ASSERT(r, effect);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
