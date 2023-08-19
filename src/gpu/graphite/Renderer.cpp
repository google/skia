/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

static uint32_t next_id() {
    static std::atomic<int32_t> nextID{0};
    // Not worried about overflow since each Context won't have that many RenderSteps, so even if
    // it wraps back to 0, that RenderStep will not be in the same Context as the original 0.
    return nextID.fetch_add(1, std::memory_order_relaxed);
}

RenderStep::RenderStep(std::string_view className,
               std::string_view variantName,
               SkEnumBitMask<Flags> flags,
               std::initializer_list<Uniform> uniforms,
               PrimitiveType primitiveType,
               DepthStencilSettings depthStencilSettings,
               SkSpan<const Attribute> vertexAttrs,
               SkSpan<const Attribute> instanceAttrs,
               SkSpan<const Varying> varyings)
        : fUniqueID(next_id())
        , fFlags(flags)
        , fPrimitiveType(primitiveType)
        , fDepthStencilSettings(depthStencilSettings)
        , fUniforms(uniforms)
        , fVertexAttrs(vertexAttrs.begin(), vertexAttrs.end())
        , fInstanceAttrs(instanceAttrs.begin(), instanceAttrs.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fVertexStride(0)
        , fInstanceStride(0)
        , fName(className) {
    for (auto v : this->vertexAttributes()) {
        fVertexStride += v.sizeAlign4();
    }
    for (auto i : this->instanceAttributes()) {
        fInstanceStride += i.sizeAlign4();
    }
    if (variantName.size() > 0) {
        fName += "[";
        fName += variantName;
        fName += "]";
    }
}

Coverage RenderStep::GetCoverage(SkEnumBitMask<Flags> flags) {
    return !(flags & Flags::kEmitsCoverage) ? Coverage::kNone
           : (flags & Flags::kLCDCoverage)  ? Coverage::kLCD
                                            : Coverage::kSingleChannel;
}

float Renderer::boundsOutset(const Transform& localToDevice, const Rect& bounds) const {
    float outset = 0.0f;
    for (int i = 0; i < this->numRenderSteps(); ++i) {
        outset = std::max(outset, this->step(i).boundsOutset(localToDevice, bounds));
    }
    return outset;
}

} // namespace skgpu::graphite
