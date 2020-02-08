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
#include "include/gpu/GrContext.h"
#include "tests/Test.h"

#include <algorithm>

DEF_TEST(SkRuntimeEffectInvalidInputs, r) {
    auto test = [r](const char* hdr, const char* expected) {
        SkString src = SkStringPrintf("%s void main(float2 p, inout half4 color) {}", hdr);
        auto[effect, errorText] = SkRuntimeEffect::Make(src);
        REPORTER_ASSERT(r, !effect);
        REPORTER_ASSERT(r, errorText.contains(expected),
                        "Expected error message to contain \"%s\". Actual message: \"%s\"",
                        expected, errorText.c_str());
    };

    // Features that are only allowed in .fp files (key, in uniform, ctype, when, tracked).
    // Ensure that these fail, and the error messages contain the relevant keyword.
    test("layout(key) in bool Input;", "key");
    test("in uniform float Input;", "in uniform");
    test("layout(ctype=SkRect) float4 Input;", "ctype");
    test("in bool Flag; layout(when=Flag) uniform float Input;", "when");
    test("layout(tracked) uniform float Input;", "tracked");

    // Runtime SkSL supports a limited set of uniform types. No samplers, for example:
    test("uniform sampler2D s;", "sampler2D");

    // 'in' variables can't be arrays
    test("in int Input[2];", "array");

    // Type specific restrictions:

    // 'bool', 'int' can't be 'uniform'
    test("uniform bool Input;", "'uniform'");
    test("uniform int Input;", "'uniform'");

    // vector and matrix types can't be 'in'
    test("in float2 Input;", "'in'");
    test("in half3x3 Input;", "'in'");
}

// Our packing rules and unit test code here relies on this:
static_assert(sizeof(bool) == 1);

class TestEffect {
public:
    TestEffect(skiatest::Reporter* r, const char* hdr, const char* body) {
        SkString src = SkStringPrintf("%s void main(float2 p, inout half4 color) { %s }",
                                      hdr, body);
        auto[effect, errorText] = SkRuntimeEffect::Make(src);
        if (!effect) {
            REPORT_FAILURE(r, "effect",
                           SkStringPrintf("Effect didn't compile: %s", errorText.c_str()));
            return;
        }

        fEffect = std::move(effect);
        fInputs = SkData::MakeUninitialized(fEffect->inputSize());
    }

    struct InputVar {
        template <typename T> InputVar& operator=(const T& val) {
            SkASSERT(sizeof(T) == fVar.sizeInBytes());
            memcpy(SkTAddOffset<void>(fOwner->fInputs->writable_data(), fVar.fOffset), &val,
                   sizeof(T));
            return *this;
        }
        TestEffect* fOwner;
        const SkRuntimeEffect::Variable& fVar;
    };

    InputVar operator[](const char* name) {
        auto input = std::find_if(fEffect->inputs().begin(), fEffect->inputs().end(),
                                  [name](const auto& v) { return v.fName.equals(name); });
        SkASSERT(input != fEffect->inputs().end());
        return {this, *input};
    }

    void test(skiatest::Reporter* r, sk_sp<SkSurface> surface,
              uint32_t TL, uint32_t TR, uint32_t BL, uint32_t BR) {
        if (!fEffect) { return; }

        auto shader = fEffect->makeShader(fInputs, nullptr, 0, nullptr, false);
        if (!shader) {
            REPORT_FAILURE(r, "shader", SkString("Effect didn't produce a shader"));
            return;
        }

        SkPaint paint;
        paint.setShader(std::move(shader));
        paint.setBlendMode(SkBlendMode::kSrc);
        surface->getCanvas()->drawPaint(paint);

        uint32_t actual[4];
        SkImageInfo info = surface->imageInfo();
        if (!surface->readPixels(info, actual, info.minRowBytes(), 0, 0)) {
            REPORT_FAILURE(r, "readPixels", SkString("readPixels failed"));
            return;
        }

        uint32_t expected[4] = {TL, TR, BL, BR};
        if (memcmp(actual, expected, sizeof(actual)) != 0) {
            REPORT_FAILURE(r, "Runtime effect didn't match expectations",
                           SkStringPrintf("\n"
                                          "Expected: [ %08x %08x %08x %08x ]\n"
                                          "Got     : [ %08x %08x %08x %08x ]\n"
                                          "SkSL:\n%s\n",
                                          TL, TR, BL, BR, actual[0], actual[1], actual[2],
                                          actual[3], fEffect->source().c_str()));
        }
    }

    void test(skiatest::Reporter* r, sk_sp<SkSurface> surface, uint32_t expected) {
        this->test(r, surface, expected, expected, expected, expected);
    }

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fInputs;
};

static void test_RuntimeEffect_Shaders(skiatest::Reporter* r, GrContext* context) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface;
    if (context) {
        surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
    } else {
        surface = SkSurface::MakeRaster(info);
    }
    REPORTER_ASSERT(r, surface);

    TestEffect xy(r, "", "color = half4(half2(p - 0.5), 0, 1);");
    xy.test(r, surface, 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);

    using float4 = std::array<float, 4>;

    // NOTE: For now, we always emit valid premul colors, until CPU and GPU agree on clamping
    TestEffect uniformColor(r, "uniform float4 gColor;", "color = half4(gColor);");

    uniformColor["gColor"] = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
    uniformColor.test(r, surface, 0xFFBF4000);

    uniformColor["gColor"] = float4{ 0.75f, 0.25f, 0.0f, 1.0f };
    uniformColor.test(r, surface, 0xFF0040BF);

    TestEffect pickColor(r, "in int flag; uniform half4 gColors[2];", "color = gColors[flag];");
    pickColor["gColors"] =
            std::array<float4, 2>{float4{1.0f, 0.0f, 0.0f, 0.498f}, float4{0.0f, 1.0f, 0.0f, 1.0f}};
    pickColor["flag"] = 0;
    pickColor.test(r, surface, 0x7F00007F);  // Tests that we clamp to valid premul
    pickColor["flag"] = 1;
    pickColor.test(r, surface, 0xFF00FF00);
}

DEF_TEST(SkRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, nullptr);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRuntimeEffectSimple_GPU, r, ctxInfo) {
    test_RuntimeEffect_Shaders(r, ctxInfo.grContext());
}
