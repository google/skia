/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"

#include "fuzz/Fuzz.h"
// call SkData::MakeSubset to divide bytes into two equal size data
// one for SkRuntimeEffect, the other for makeShader
bool FuzzSKSL2Pipeline(sk_sp<SkData> bytes) {
    size_t half = bytes->size() / 2;

    sk_sp<SkData> effectBytes = SkData::MakeSubset(bytes.get(), 0, half);
    sk_sp<SkData> shaderBytes = SkData::MakeSubset(bytes.get(), half, half);

    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::Program::kPipelineStage_Kind,
                                                    SkSL::String((const char*) effectBytes->data(),
                                                                 effectBytes->size()),
                                                    settings);
    SkSL::PipelineStageArgs args;
    if (!program || !compiler.toPipelineStage(*program, &args)) {
        return false;
    }
    SkRuntimeEffect::EffectResult pair = SkRuntimeEffect::Make(
        SkString((const char*) effectBytes->data(), effectBytes->size())
    );
    SkRuntimeEffect* effect = std::get<0>(pair).get();
    if (!effect) {
        return false;
    }

    SkMatrix localM;
    localM.setRotate(90, 128, 128);
    auto shader = effect->makeShader(shaderBytes, nullptr, 0, &localM, true);
    if (!shader) {
        return false;
    }
    SkPaint paint;
    paint.setShader(std::move(shader));

    sk_sp<SkSurface> s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        return false;
    }

    s->getCanvas()->drawPaint(paint);

    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Pipeline(bytes);
    return 0;
}
#endif
