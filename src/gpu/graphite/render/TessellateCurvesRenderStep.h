/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateCurvesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateCurvesRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class StaticBufferManager;

class TessellateCurvesRenderStep final : public RenderStep {
public:
    // TODO: If this takes DepthStencilSettings directly and a way to adjust the flags to specify
    // that it performs shading, this RenderStep definition can be shared between the stencil and
    // the convex rendering variants.
    TessellateCurvesRenderStep(bool evenOdd,
                               bool infinitySupport,
                               StaticBufferManager* bufferManager);

    ~TessellateCurvesRenderStep() override;

    std::string vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, skvx::ushort2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    // Points to the static buffers holding the fixed indexed vertex template for drawing instances.
    BindBufferInfo fVertexBuffer;
    BindBufferInfo fIndexBuffer;
    bool fInfinitySupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateCurvesRenderStep_DEFINED
