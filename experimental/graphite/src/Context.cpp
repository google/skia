/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Recording.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

namespace skgpu {

namespace {

#if 0

// From MtlToy
struct FragmentUniforms {
    vector_float2 startPos;
    vector_float2 endPos;

    vector_float4 color0;
    vector_float4 color1;
    vector_float4 color2;
    vector_float4 color3;

    vector_float4 stops;
    simd_uint2 rtSize;
};

#endif

// TODO: convert to build_SKSL
void build_MSL(const Combination &combo) {
#if 0
    fragment float4 fragmentShader(VertexOut interpolated [[stage_in]],
    constant FragmentUniforms &uniforms [[buffer(0)]])
    {
        float2 screenPos = float2(2*interpolated.pos.x/uniforms.rtSize[0] - 1,
                                  2*interpolated.pos.y/uniforms.rtSize[1] - 1);
        float2 delta = uniforms.endPos - uniforms.startPos;
        float2 pt = screenPos - uniforms.startPos;
        float t = dot(pt, delta) / dot(delta, delta);
        float4 result = uniforms.color0;
        result = mix(result, uniforms.color1, clamp((t-uniforms.stops.x)/(uniforms.stops.y-uniforms.stops.x),
                                                    0.0, 1.0));
        result = mix(result, uniforms.color2, clamp((t-uniforms.stops.y)/(uniforms.stops.z-uniforms.stops.y),
                                                    0.0, 1.0));
        result = mix(result, uniforms.color3, clamp((t-uniforms.stops.z)/(uniforms.stops.w-uniforms.stops.z),
                                                    0.0, 1.0));
        return result;
//      return float4(uniforms.color1);
#endif
}

// TODO: Going forward this will change into:
//    a method that takes a single combination and builds the SkSL (i.e., buildSKSL(combo))
//    a method that extracts a combination from an SkPaint (i.e., ExtractCombo(SkPaint))
// buildSKSL will have to collect the number of uniforms and their types required by the SkSL
// extractCombo will have to collect the actual uniforms
void expand(const PaintCombo& c) {
    for (auto bm: c.fBlendModes) {
        for (auto& shaderCombo: c.fShaders) {
            for (auto shaderType: shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    build_MSL({shaderType, tm, bm});
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
