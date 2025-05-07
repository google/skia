/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

RenderStep::RenderStep(RenderStepID renderStepID,
                       SkEnumBitMask<Flags> flags,
                       std::initializer_list<Uniform> uniforms,
                       PrimitiveType primitiveType,
                       DepthStencilSettings depthStencilSettings,
                       SkSpan<const Attribute> staticAttrs,
                       SkSpan<const Attribute> appendAttrs,
                       SkSpan<const Varying> varyings)
        : fRenderStepID(renderStepID)
        , fFlags(flags)
        , fPrimitiveType(primitiveType)
        , fDepthStencilSettings(depthStencilSettings)
        , fUniforms(uniforms)
        , fStaticAttrs(staticAttrs.begin(), staticAttrs.end())
        , fAppendAttrs(appendAttrs.begin(), appendAttrs.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fStaticDataStride(0)
        , fAppendDataStride(0) {
    for (auto v : this->staticAttributes()) {
        fStaticDataStride += v.sizeAlign4();
    }
    for (auto i : this->appendAttributes()) {
        fAppendDataStride += i.sizeAlign4();
    }
}

Coverage RenderStep::GetCoverage(SkEnumBitMask<Flags> flags) {
    return !(flags & Flags::kEmitsCoverage) ? Coverage::kNone
           : (flags & Flags::kLCDCoverage)  ? Coverage::kLCD
                                            : Coverage::kSingleChannel;
}

const char* RenderStep::RenderStepName(RenderStepID id) {
#define CASE1(BaseName) case RenderStepID::k##BaseName: return #BaseName "RenderStep";
#define CASE2(BaseName, VariantName) \
    case RenderStepID::k##BaseName##_##VariantName: return #BaseName "RenderStep[" #VariantName "]";

    switch (id) {
        SKGPU_RENDERSTEP_TYPES(CASE1, CASE2)
    }
#undef CASE1
#undef CASE2

    SkUNREACHABLE;
}

bool RenderStep::IsValidRenderStepID(uint32_t renderStepID) {
    return renderStepID > (int) RenderStep::RenderStepID::kInvalid &&
           renderStepID < RenderStep::kNumRenderSteps;
}

} // namespace skgpu::graphite
