/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <string>
#include <utility>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
class StaticBufferManager;
struct DepthStencilSettings;

class TessellateWedgesRenderStep final : public RenderStep {
public:
    // 'vertexBuffer' and 'indexBuffer' must have been returned by CreateVertexTemplate(), but they
    // can be shared by all instances of TessellateWedgesRenderStep.
    TessellateWedgesRenderStep(RenderStepID renderStepID,
                               bool infinitySupport,
                               DepthStencilSettings depthStencilSettings,
                               StaticBufferManager* bufferManager);

    ~TessellateWedgesRenderStep() override;

    static std::pair<BindBufferInfo, BindBufferInfo> CreateVertexTemplate(StaticBufferManager*);

    std::string vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, skvx::uint2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    // Points to the static buffers holding the fixed indexed vertex template for drawing instances.
    BindBufferInfo fVertexBuffer;
    BindBufferInfo fIndexBuffer;
    bool fInfinitySupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
