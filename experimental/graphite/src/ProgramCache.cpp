/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

namespace {

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == skgpu::ProgramCache::kInvalidProgramID);
    return id;
}

} // anonymous namespace

namespace skgpu {

////////////////////////////////////////////////////////////////////////////////////////////////////

ProgramCache::ProgramInfo::ProgramInfo(Combination c)
    : fID(next_id())
    , fCombination(c) {
}

ProgramCache::ProgramInfo::~ProgramInfo() {}

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

std::string ProgramCache::ProgramInfo::getMSL() const {
    switch (fCombination.fShaderType) {
        case ShaderCombo::ShaderType::kLinearGradient:
            return std::string(
            "float2 screenPos = float2(2*interpolated.pos.x/uniforms.rtSize[0] - 1,"
            "                          2*interpolated.pos.y/uniforms.rtSize[1] - 1);"
            "float2 delta = uniforms.endPos - uniforms.startPos;"
            "float2 pt = screenPos - uniforms.startPos;"
            "float t = dot(pt, delta) / dot(delta, delta);"
            "float4 result = uniforms.color0;"
            "result = mix(result, uniforms.color1,"
            "             clamp((t-uniforms.stops.x)/(uniforms.stops.y-uniforms.stops.x), 0, 1));"
            "result = mix(result, uniforms.color2,"
            "             clamp((t-uniforms.stops.y)/(uniforms.stops.z-uniforms.stops.y), 0, 1));"
            "result = mix(result, uniforms.color3,"
            "             clamp((t-uniforms.stops.z)/(uniforms.stops.w-uniforms.stops.z), 0, 1));"
            "return result;");
        case ShaderCombo::ShaderType::kRadialGradient:
        case ShaderCombo::ShaderType::kSweepGradient:
        case ShaderCombo::ShaderType::kConicalGradient:
        case ShaderCombo::ShaderType::kNone:
        default:
            return std::string("return float4(uniforms.color1);");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t ProgramCache::Hash::operator()(Combination c) const {
    return static_cast<int>(c.fShaderType) +
           static_cast<int>(c.fTileMode) +
           static_cast<int>(c.fBlendMode);
}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::findOrCreateProgram(Combination c) {
    auto iter = fPrograms.find(c);
    if (iter != fPrograms.end()) {
        SkASSERT(iter->second->id() != kInvalidProgramID);
        return iter->second;
    }

    sk_sp<ProgramInfo> pi(new ProgramInfo(c));
    fPrograms.insert(std::make_pair(c, pi));
    return pi;
}

} // namespace skgpu
