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
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "src/sksl/SkSLCompiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <thread>


//FIXME REMOVE
#include "src/sksl/dsl/priv/DSLWriter.h"

using namespace SkSL::dsl;

class DSLTestEffect {
public:
    DSLTestEffect(skiatest::Reporter* r, sk_sp<SkSurface> surface)
        : fReporter(r)
        , fCaps(SkSL::ShaderCapsFactory::Standalone())
        , fCompiler(new SkSL::Compiler(fCaps.get()))
        , fSurface(std::move(surface)) {}

    void start() {
        StartRuntimeEffect(fCompiler.get());
    }

    void end() {
        sk_sp<SkRuntimeEffect> effect = EndRuntimeEffect();
        SkASSERT(effect);
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
    DSLTestEffect effect(r, surface);

    // Local coords
    {
        effect.start();
        Var p(kFloat2, "p");
        Function(kHalf4, "main", p).define(
            Return(Half4(Half2(p - 0.5), 0, 1))
        );
        effect.end();
        effect.test(0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF);
    }

    // Use of a simple uniform. (Draw twice with two values to ensure it's updated).
    {
        effect.start();
        Var gColor(kUniform_Modifier, kFloat4);
        Var p(kFloat2, "p");
        Function(kHalf4, "main", p).define(
            Return(Half4(gColor))
        );
        effect.end();
        effect.uniform(gColor.name()) = float4{ 0.0f, 0.25f, 0.75f, 1.0f };
        effect.test(0xFFBF4000);
        effect.uniform(gColor.name()) = float4{ 1.0f, 0.0f, 0.0f, 0.498f };
        effect.test(0x7F00007F);  // Tests that we clamp to valid premul
    }
}

DEF_TEST(DSLRuntimeEffectSimple, r) {
    test_RuntimeEffect_Shaders(r, nullptr);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DSLRuntimeEffectSimple_GPU, r, ctxInfo) {
    test_RuntimeEffect_Shaders(r, ctxInfo.directContext());
}
