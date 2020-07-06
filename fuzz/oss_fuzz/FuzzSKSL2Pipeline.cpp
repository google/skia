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

bool FuzzSKSL2Pipeline(sk_sp<SkData> bytes) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::Program::kPipelineStage_Kind,
                                                    SkSL::String((const char*) bytes->data(),
                                                                 bytes->size()),
                                                    settings);
    SkSL::PipelineStageArgs args;
    if (!program || !compiler.toPipelineStage(*program, &args)) {
        return false;
    }

    SkRuntimeEffect::EffectResult pair = SkRuntimeEffect::Make(
        SkSL::String((const char*) bytes->data(), bytes->size())
    );
    SkRuntimeEffect* effect = std::get<0>(pair).get();

    if (!effect) {
        return false;
    }

    SkMatrix localM;
    localM.setRotate(90, 128, 128);
    sk_sp<SkShader> shader = effect->makeShader(bytes, nullptr, 0, &localM, true);

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
    s->getCanvas()->drawRect({0, 0, 256, 256}, paint);
    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Pipeline(bytes);
    return 0;
}
#endif
