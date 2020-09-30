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
    auto test = [r](const char* hdr, const char* body, const char* expected) {
        SkString src = SkStringPrintf("%s half4 main(float2 p) { %s return half4(0); }",
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

    // Runtime SkSL supports a limited set of uniform types. No samplers, bool, or int, for example:
    test("uniform sampler2D s;", "", "uniform");
    test("uniform bool b;", "", "uniform");
    test("uniform int i;", "", "uniform");

    // 'in' variables aren't allowed at all:
    test("in bool b;", "", "'in'");
    test("in float f;", "", "'in'");
    test("in float2 v;", "", "'in'");
    test("in half3x3 m;", "", "'in'");

    // 'marker' is only permitted on float4x4 uniforms
    test("layout(marker=local_to_world) uniform float3x3 localToWorld;", "", "float4x4");

    test("float missing();", "p.x = missing();", "undefined function");

    // Shouldn't be possible to create an SkRuntimeEffect without "main"
    test("//", "", "main");

    // Various places that shaders (fragmentProcessors) should not be allowed
    test("",
         "shader child;",
         "must be global");
    test("in shader child; half4 helper(shader fp) { return sample(fp); }",
         "half4 color = helper(child);",
         "parameter");
    test("in shader child; shader get_child() { return child; }",
         "half4 color = sample(get_child());",
         "return");
    test("in shader child;",
         "half4 color = sample(shader(child));",
         "construct");
    test("in shader child1; in shader child2;",
         "half4 color = sample(p.x > 10 ? child1 : child2);",
         "expression");

    // Errors that aren't caught until later in the compilation process (during optimize())
    test("", "return half4(1);", "unreachable");
    test("half badFunc() { }", "", "without returning");
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

    void build(const char* header, const char* body) {
        SkString src = SkStringPrintf("%s half4 main(float2 p) { %s }",
                                      header, body);
        auto[effect, errorText] = SkRuntimeEffect::Make(src);
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
    return bmp.makeShader();
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
    effect.build("", "return half4(half2(p - 0.5), 0, 1);");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    effect.build("uniform float4 gColor;",
                 "return half4(gColor);");
    effect.uniform("gColor") = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
    effect.test(0xFFBF4000);
    effect.uniform("gColor") = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
    effect.test(0x7F00007F);  // Tests that we clamp to valid premul

    // Test sk_FragCoord (device coords). Rotate the canvas to be sure we're seeing device coords.
    // Since the surface is 2x2, we should see (0,0), (1,0), (0,1), (1,1). Multiply by 0.498 to
    // make sure we're not saturating unexpectedly.
    effect.build("", "return half4(0.498 * (half2(sk_FragCoord.xy) - 0.5), 0, 1);");
    effect.test(0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F,
                [](SkCanvas* canvas, SkPaint*) { canvas->rotate(45.0f); });

    // Runtime effects should use relaxed precision rules by default
    effect.build("", "return float4(p - 0.5, 0, 1);");
    effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    //
    // Sampling children
    //

    // Sampling a null child should return the paint color
    effect.build("in shader child;",
                 "return sample(child);");
    effect.child("child") = nullptr;
    effect.test(0xFF00FFFF,
                [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });

    sk_sp<SkShader> rgbwShader = make_RGBW_shader();

    // Sampling a simple child at our coordinates (implicitly)
    effect.build("in shader child;",
                 "return sample(child);");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF);

    // Sampling with explicit coordinates (reflecting about the diagonal)
    effect.build("in shader child;",
                 "return sample(child, p.yx);");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFFFF);

    // Sampling with a matrix (again, reflecting about the diagonal)
    effect.build("in shader child;",
                 "return sample(child, float3x3(0, 1, 0, 1, 0, 0, 0, 0, 1));");
    effect.child("child") = rgbwShader;
    effect.test(0xFF0000FF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFFFF);

    //
    // Helper functions
    //

    // Test case for inlining in the pipeline-stage and fragment-shader passes (skbug.com/10526):
    effect.build("float2 helper(float2 x) { return x + 1; }",
                 "float2 v = helper(p);"
                 "return half4(half2(v), 0, 1);");
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
