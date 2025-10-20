/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

#include "src/gpu/graphite/DrawParams.h"

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

std::optional<SkIRect> RenderStep::getScissor(const DrawParams& params,
                                              SkIRect currentScissor,
                                              SkIRect deviceBounds) const {
    if (currentScissor == params.scissor()) {
        return {}; // Trivially no change in scissor state is required
    }

    Rect drawBounds = params.drawBounds();
    if (params.geometry().isShape() && params.geometry().shape().inverted()) {
        // For inverse filled shapes, the scissor is able to be handled in unique ways.
        if (fFlags & Flags::kInverseFillsScissor) {
            // In this case, the RenderStep geometrically respects the scissor so as long as the
            // current scissor doesn't interfere, we don't need a state change.
            if (currentScissor.contains(params.scissor())) {
                return {};
            } else {
                // This draw doesn't need a scissor at all, so return the device bounds. It is
                // expected that this will generally lead to fewer scissor state changes (for
                // instance when applying the cover steps for a lot of inverse-filled intersect
                // clip depth-only draws). However, it could lead to a redundant scissor change if
                // the next draw would have used this draw's original scissor.
                return deviceBounds;
            }
        }

        if (fFlags & Flags::kIgnoreInverseFill) {
            // In this case params.drawBounds() fills the scissor from the inverse fill rule,
            // but we want to apply the scissor as if it were a regular fill.
            drawBounds = params.transformedShapeBounds(); // this ignores fill rule
            drawBounds.intersect(params.scissor());
        } // Else leave drawBounds filling the original scissor

        // Fall through to regular scissor state checking with the possibly-updated bounds
    }

    // Draws that are unaffected by a clip stack will have a scissor matching the device's bounds.
    // If their transformed shape bounds clipped to the current scissor are no different than their
    // draw bounds (clipped to the original scissor), then no state change is required.
    Rect currentClippedBounds = params.transformedShapeBounds();
    currentClippedBounds.intersect(currentScissor);
    if (currentClippedBounds == drawBounds) {
        return {};
    }

    if (drawBounds == params.transformedShapeBounds()) {
        // We need to change the scissor, but the registered scissor is a no-op so
        // use the device bounds as a canonical scissor.
        return deviceBounds;
    }

    return params.scissor();
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
