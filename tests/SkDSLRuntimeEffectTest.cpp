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
#include "include/sksl/DSLRuntimeEffects.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "src/sksl/SkSLCompiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <thread>

using namespace SkSL::dsl;

class DSLTestEffect {
public:
    DSLTestEffect(skiatest::Reporter* r, sk_sp<SkSurface> surface)
        : fReporter(r)
        , fCaps(SkSL::ShaderCapsFactory::Standalone())
        , fCompiler(std::make_unique<SkSL::Compiler>(fCaps.get()))
        , fSurface(std::move(surface)) {}

    void start() {
        StartRuntimeShader(fCompiler.get());
    }

    void end(bool expectSuccess = true) {
        SkRuntimeEffect::Options options;
        SkRuntimeEffectPriv::EnableFragCoord(&options);
        sk_sp<SkRuntimeEffect> effect = EndRuntimeShader(options);
        REPORTER_ASSERT(fReporter, effect ? expectSuccess : !expectSuccess);
        if (effect) {
            fBuilder.init(std::move(effect));
        }
    }

    SkRuntimeShaderBuilder::BuilderUniform uniform(skstd::string_view name) {
        return fBuilder->uniform(SkString(name).c_str());
    }

    SkRuntimeShaderBuilder::BuilderChild child(skstd::string_view name) {
        return fBuilder->child(SkString(name).c_str());
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
    SkSL::ShaderCapsPointer         fCaps;
    std::unique_ptr<SkSL::Compiler> fCompiler;
    sk_sp<SkSurface>                fSurface;
    SkTLazy<SkRuntimeShaderBuilder> fBuilder;
};

static void test_RuntimeEffect_Shaders(skiatest::Reporter* r, GrRecordingContext* rContext) {
    SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = rContext
                                    ? SkSurface::MakeRenderTarget(rContext, SkBudgeted::kNo, info)
                                    : SkSurface::MakeRaster(info);
    REPORTER_ASSERT(r, surface);
    using float4 = std::array<float, 4>;
    using int4 = std::array<int, 4>;
    DSLTestEffect effect(r, surface);

    // Local coords
    {
        effect.start();
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(Half4(Half2(p - 0.5), 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    {
        effect.start();
        GlobalVar gColor(kUniform_Modifier, kFloat4_Type);
        Declare(gColor);
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(Half4(gColor))
        );
        effect.end();
        effect.uniform(SkString(gColor.name()).c_str()) = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
        effect.test(0xFFBF4000);
        effect.uniform(SkString(gColor.name()).c_str()) = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
        effect.test(0x7F0000FF);  // Tests that we don't clamp to valid premul
    }

    // Same, with integer uniforms
    {
        effect.start();
        GlobalVar gColor(kUniform_Modifier, kInt4_Type);
        Declare(gColor);
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(Half4(gColor) / 255)
        );
        effect.end();
        effect.uniform(SkString(gColor.name()).c_str()) = int4{ 0x00, 0x40, 0xBF, 0xFF };
        effect.test(0xFFBF4000);
        effect.uniform(SkString(gColor.name()).c_str()) = int4{ 0xFF, 0x00, 0x00, 0x7F };
        effect.test(0x7F0000FF);  // Tests that we don't clamp to valid premul
    }

    // Test sk_FragCoord (device coords). Rotate the canvas to be sure we're seeing device coords.
    // Since the surface is 2x2, we should see (0,0), (1,0), (0,1), (1,1). Multiply by 0.498 to
    // make sure we're not saturating unexpectedly.
    {
        effect.start();
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(Half4(0.498 * (Half2(Swizzle(sk_FragCoord(), X, Y)) - 0.5), 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF00007F, 0xFF007F00, 0xFF007F7F,
                    [](SkCanvas* canvas, SkPaint*) { canvas->rotate(45.0f); });
    }

    // Runtime effects should use relaxed precision rules by default
    {
        effect.start();
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(Float4(p - 0.5, 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }

    // ... and support *returning* float4, not just half4
    {
        effect.start();
        Parameter p(kFloat2_Type, "p");
        Function(kFloat4_Type, "main", p).define(
            Return(Float4(p - 0.5, 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }

    // Test error reporting. We put this before a couple of successful tests to ensure that a
    // failure doesn't leave us in a broken state.
    {
        class SimpleErrorReporter : public SkSL::ErrorReporter {
        public:
            void handleError(skstd::string_view msg, SkSL::PositionInfo pos) override {
                fMsg += msg;
            }

            SkSL::String fMsg;
        } errorReporter;
        effect.start();
        SetErrorReporter(&errorReporter);
        Parameter p(kFloat2_Type, "p");
        Function(kHalf4_Type, "main", p).define(
            Return(1) // Error, type mismatch
        );
        effect.end(false);
        REPORTER_ASSERT(r, errorReporter.fMsg == "expected 'half4', but found 'int'");
    }

    // Mutating coords should work. (skbug.com/10918)
    {
        effect.start();
        Parameter p(kFloat2_Type, "p");
        Function(kFloat4_Type, "main", p).define(
            p -= 0.5,
            Return(Float4(p, 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }
    {
        effect.start();
        Parameter p1(kInOut_Modifier, kFloat2_Type, "p");
        Function moveCoords(kVoid_Type, "moveCoords", p1);
        moveCoords.define(
            p1 -= 0.5
        );
        Parameter p2(kFloat2_Type, "p");
        Function(kFloat4_Type, "main", p2).define(
            moveCoords(p2),
            Return(Float4(p2, 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }

    //
    // Sampling children
    //

    // Sampling a null child should return the paint color
    {
        effect.start();
        GlobalVar child(kUniform_Modifier, kShader_Type, "child");
        Declare(child);
        Parameter p2(kFloat2_Type, "p");
        Function(kFloat4_Type, "main", p2).define(
            Return(child.eval(p2))
        );
        effect.end();
        effect.child(child.name()) = nullptr;
        effect.test(0xFF00FFFF,
                    [](SkCanvas*, SkPaint* paint) { paint->setColor4f({1.0f, 1.0f, 0.0f, 1.0f}); });
    }
}

DEF_TEST(DSLRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, nullptr);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DSLRuntimeEffectSimple_GPU, r, ctxInfo) {
    test_RuntimeEffect_Shaders(r, ctxInfo.directContext());
}
