/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipelineDesc_DEFINED
#define skgpu_graphite_GraphicsPipelineDesc_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc() : fRenderStepID(0), fPaintID(UniquePaintParamsID::InvalidID()) {}
    GraphicsPipelineDesc(const RenderStep* renderStep, UniquePaintParamsID paintID)
        : fRenderStepID(renderStep->uniqueID())
        , fPaintID(paintID) {}

    bool operator==(const GraphicsPipelineDesc& that) const {
        return fRenderStepID == that.fRenderStepID && fPaintID == that.fPaintID;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    // Describes the geometric portion of the pipeline's program and the pipeline's fixed state
    // (except for renderpass-level state that will never change between draws).
    uint32_t renderStepID() const { return fRenderStepID; }
    // UniqueID of the required PaintParams
    UniquePaintParamsID paintParamsID() const { return fPaintID; }

private:
    // Each RenderStep defines a fixed set of attributes and rasterization state, as well as the
    // shader fragments that control the geometry and coverage calculations. The RenderStep's shader
    // is combined with the rest of the shader generated from the PaintParams. Because each
    // RenderStep is fixed, its pointer can be used as a proxy for everything that it specifies in
    // the GraphicsPipeline.
    uint32_t fRenderStepID;
    UniquePaintParamsID fPaintID;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipelineDesc_DEFINED
