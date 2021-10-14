/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Recording.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

// TODO: these are only included to play with shader combinations
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

namespace skgpu {

namespace {

// TODO: Going forward this will change into:
//    a method that takes a single combination and builds the SkSL (i.e., buildSKSL(combo))
//    a method that extracts a combination from an SkPaint (i.e., extractCombo(SkPaint))
// buildSKSL will have to collect the number of uniforms and their types required by the SkSL
// extractCombo will have to collect the actual uniforms
void expand(const PaintCombo& c) {
    SkPoint pts[2] = { { -100, -100 }, { 100, 100 } };
    SkColor colors[2] = { SK_ColorRED, SK_ColorGREEN };

    for (auto bm : c.fBlendModes) {
        for (auto& shaderCombo : c.fShaders) {
            for (auto shaderType : shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    sk_sp<SkShader> s;
                    switch (shaderType) {
                        case ShaderCombo::ShaderType::kNone:
                            break;
                        case ShaderCombo::ShaderType::kLinearGradient:
                            s = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, tm);
                            break;
                        case ShaderCombo::ShaderType::kRadialGradient:
                            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, nullptr, 2, tm);
                            break;
                        case ShaderCombo::ShaderType::kSweepGradient:
                            s = SkGradientShader::MakeSweep(0, 0, colors, nullptr, 2);
                            break;
                        case ShaderCombo::ShaderType::kConicalGradient:
                            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                                      {-100, -100}, 100,
                                                                      colors, nullptr, 2, tm);
                            break;
                    }
                    SkPaint p;
                    p.setShader(std::move(s));
                    p.setBlendMode(bm);
                }
            }
        }
    }
}

} // anonymous namespace

Context::Context(sk_sp<Gpu> gpu) : fGpu(std::move(gpu)) {}
Context::~Context() {}

#ifdef SK_METAL
sk_sp<Context> Context::MakeMetal(const mtl::BackendContext& backendContext) {
    sk_sp<Gpu> gpu = mtl::Trampoline::MakeGpu(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return sk_sp<Context>(new Context(std::move(gpu)));
}
#endif


sk_sp<Recorder> Context::createRecorder() {
    return sk_make_sp<Recorder>(sk_ref_sp(this));
}

void Context::insertRecording(std::unique_ptr<Recording> recording) {
    fRecordings.emplace_back(std::move(recording));
}

void Context::submit(SyncToCpu syncToCpu) {
    // TODO: we want Gpu::submit to take an array of command buffers but, for now, it just takes
    // one. Once we have more than one recording queued up we will need to extract the
    // command buffers and submit them as a block.
    SkASSERT(fRecordings.size() == 1);
    fGpu->submit(fRecordings[0]->fCommandBuffer);

    fGpu->checkForFinishedWork(syncToCpu);
    fRecordings.clear();
}

void Context::preCompile(const PaintCombo& c) {
    expand(c);
}

} // namespace skgpu
