/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

namespace skgpu {

////////////////////////////////////////////////////////////////////////////////////////////////////
ProgramCache::ProgramInfo::ProgramInfo(uint32_t uniqueID, Combination c)
    : fID(uniqueID)
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
ProgramCache::ProgramCache() {
    // kInvalidProgramID (aka 0) is reserved
    fProgramVector.push_back(nullptr);
}

size_t ProgramCache::Hash::operator()(Combination c) const {
    return static_cast<int>(c.fShaderType) +
           static_cast<int>(c.fTileMode) +
           static_cast<int>(c.fBlendMode);
}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::findOrCreateProgram(Combination c) {
    auto iter = fProgramHash.find(c);
    if (iter != fProgramHash.end()) {
        SkASSERT(iter->second->id() != kInvalidProgramID);
        return iter->second;
    }

    sk_sp<ProgramInfo> pi(new ProgramInfo(fNextUniqueID++, c));
    fProgramHash.insert(std::make_pair(c, pi));
    fProgramVector.push_back(pi);
    SkASSERT(fProgramVector[pi->id()] == pi);
    return pi;
}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fProgramVector.size());
    return fProgramVector[uniqueID];
}

} // namespace skgpu
