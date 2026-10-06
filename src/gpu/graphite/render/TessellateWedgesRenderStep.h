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

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
class StaticBufferManager;

class TessellateWedgesRenderStep final : public RenderStep {
public:
    // Three variants of TessellateWedgesRenderStep differ in their depth/stencil settings and
    // whether or not they are a stencil pre-pass or a standalone step.
    static std::unique_ptr<TessellateWedgesRenderStep> StencilFill(
            Layout, bool evenOdd, bool infinitySupport, StaticBufferManager*);

    static std::unique_ptr<TessellateWedgesRenderStep> ConvexFill(
            Layout, bool infinitySupport, StaticBufferManager*);

    ~TessellateWedgesRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    TessellateWedgesRenderStep(Layout, RenderStepID, bool infinitySupport,
                               SkEnumBitMask<Flags> xtraFlags, DepthStencilSettings,
                               StaticBufferManager*);

    // Points to the static buffers holding the fixed indexed vertex template for drawing instances.
    BindBufferInfo fVertexBuffer;
    BindBufferInfo fIndexBuffer;
    bool fInfinitySupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
