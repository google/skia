/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/effects/GrSkSLFP.h"

struct FpAndKey {
    std::unique_ptr<GrFragmentProcessor> fp;
    SkTArray<uint32_t, true>             key;
};

DEF_SIMPLE_GPU_GM(runtimefpspecialized, ctx, sdCtx, canvas, 232, 232) {
    // Constant color, but with a similar option to GrOverrideInputFragmentProcessor
    // useUniform decides if the color is treated as a uniform, or a literal inserted in the SkSL
    auto make_color_fp = [&](SkPMColor4f color, bool useUniform) {
        auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
            uniform half4 color;
            half4 main(float2 xy) { return color; }
        )");
        FpAndKey result;
        if (useUniform) {
            result.fp = GrSkSLFP::Make(std::move(effect), "color_fp", "color", color);
        } else {
            result.fp = GrSkSLFP::Make(std::move(effect), "color_fp", "color", Specialize(color));
        }
        GrProcessorKeyBuilder builder(&result.key);
        result.fp->getGLSLProcessorKey(*sdCtx->caps()->shaderCaps(), &builder);
        return result;
    };

    int x = 10, y = 10;

    auto nextCol = [&] { x += (64 + 10); };
    auto nextRow = [&] { x = 10; y += (64 + 10); };

    auto draw = [&](std::unique_ptr<GrFragmentProcessor> fp) {
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        sdCtx->drawRect(nullptr,
                        std::move(paint),
                        GrAA::kNo,
                        SkMatrix::Translate(x, y),
                        SkRect::MakeIWH(64, 64));
        nextCol();
    };

    auto uRed   = make_color_fp({1, 0, 0, 1}, true),
         uGreen = make_color_fp({0, 1, 0, 1}, true),
         sRed   = make_color_fp({1, 0, 0, 1}, false),
         sGreen = make_color_fp({0, 1, 0, 1}, false);

    // uRed and uGreen should have the same key - they just have different uniforms
    SkASSERT(uRed.key == uGreen.key);
    // sRed and sGreen should have keys that are different from the uniform case, and each other
    SkASSERT(sRed.key != uRed.key);
    SkASSERT(sGreen.key != uRed.key);
    SkASSERT(sRed.key != sGreen.key);

    draw(std::move(uRed.fp));
    draw(std::move(uGreen.fp));
    nextRow();

    draw(std::move(sRed.fp));
    draw(std::move(sGreen.fp));
}
