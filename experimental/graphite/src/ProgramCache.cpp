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

std::string ProgramCache::ProgramInfo::getMSL() const {
    std::string msl = GetMSLUniformStruct(fCombination.fShaderType);

    switch (fCombination.fShaderType) {
        case ShaderCombo::ShaderType::kLinearGradient:
            // TODO: this MSL uses a 'rtSize' uniform that, presumably, we'll be getting from the
            // vertex shader side of things (still somewhat TBD)
            msl += std::string(
            "fragment float4 fragmentShader(VertexOut interpolated [[stage_in]],\n"
            "                               constant FragmentUniforms &uniforms [[buffer(0)]])\n"
            "{"
            "float2 screenPos = float2(2*interpolated.pos.x/uniforms.rtSize[0] - 1,\n"
            "                          2*interpolated.pos.y/uniforms.rtSize[1] - 1);\n"
            "float2 delta = uniforms.point1 - uniforms.point0;\n"
            "float2 pt = screenPos - uniforms.point0;\n"
            "float t = dot(pt, delta) / dot(delta, delta);\n"
            "float4 result = uniforms.colors[0];\n"
            "result = mix(result, uniforms.colors[1],\n"
            "             clamp((t-uniforms.offsets[0])/(uniforms.offsets[1]-uniforms.offsets[0]),\n"
            "                   0, 1));\n"
            "result = mix(result, uniforms.colors[2],\n"
            "             clamp((t-uniforms.offsets[1])/(uniforms.offsets[2]-uniforms.offsets[1]),\n"
            "                   0, 1));\n"
            "result = mix(result, uniforms.colors[3],\n"
            "             clamp((t-uniforms.offsets[2])/(uniforms.offsets[3]-uniforms.offsets[2]),\n"
            "             0, 1));\n"
            "return result;\n"
            "}\n");
            break;
        case ShaderCombo::ShaderType::kNone:
            // TODO: kNone is for depth-only draws, so should actually have a fragment output type
            // that only defines a [[depth]] attribute but no color calculation.
            msl +=
                    "fragment float4 fragmentMain(VertexOutput interpolated [[stage_in]]) {\n"
                    "    return float4(0.0, 0.0, 1.0, 1.0);\n"
                    "}\n";
            break;
        case ShaderCombo::ShaderType::kRadialGradient:
        case ShaderCombo::ShaderType::kSweepGradient:
        case ShaderCombo::ShaderType::kConicalGradient:
        case ShaderCombo::ShaderType::kSolidColor:
        default:
            msl += std::string(
            "fragment float4 fragmentShader(VertexOut interpolated [[stage_in]],\n"
            "                               constant FragmentUniforms &uniforms [[buffer(0)]])\n"
            "{\n"
            "return float4(uniforms.color);\n"
            "}\n");
            break;
    }

    return msl;
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
