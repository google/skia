/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu::graphite {

class StaticBufferManager;

class TessellateWedgesRenderStep final : public RenderStep {
public:
    // 'vertexBuffer' and 'indexBuffer' must have been returned by CreateVertexTemplate(), but they
    // can be shared by all instances of TessellateWedgesRenderStep.
    TessellateWedgesRenderStep(std::string_view variantName,
                               bool infinitySupport,
                               DepthStencilSettings depthStencilSettings,
                               StaticBufferManager* bufferManager);

    ~TessellateWedgesRenderStep() override;

    static std::pair<BindBufferInfo, BindBufferInfo> CreateVertexTemplate(StaticBufferManager*);

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

#endif // skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
