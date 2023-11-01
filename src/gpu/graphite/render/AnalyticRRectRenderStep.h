/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_AnalyticRRectRenderStep_DEFINED
#define skgpu_graphite_render_AnalyticRRectRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class StaticBufferManager;

class AnalyticRRectRenderStep final : public RenderStep {
public:
    AnalyticRRectRenderStep(StaticBufferManager* bufferManager);

    ~AnalyticRRectRenderStep() override;

    std::string vertexSkSL() const override;
    const char* fragmentCoverageSkSL() const override;

    float boundsOutset(const Transform& localToDevice, const Rect& bounds) const override;

    void writeVertices(DrawWriter*, const DrawParams&, skvx::ushort2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    // Points to the static buffers holding the fixed indexed vertex template for drawing instances.
    BindBufferInfo fVertexBuffer;
    BindBufferInfo fIndexBuffer;
};

}  // namespace skgpu::graphite

#endif // skgpu_render_AnalyticRRectRenderStep_DEFINED
