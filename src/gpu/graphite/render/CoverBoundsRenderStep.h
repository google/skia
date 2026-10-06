/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_CoverBoundsRenderStep_DEFINED
#define skgpu_graphite_render_CoverBoundsRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;

class CoverBoundsRenderStep final : public RenderStep {
public:
    // Three variants of the CoverBoundsRenderStep differ in their depth/stencil settings.
    static std::unique_ptr<CoverBoundsRenderStep> StencilCover(Layout, bool inverseFill);
    static std::unique_ptr<CoverBoundsRenderStep> NonAAInnerFill(Layout);

    ~CoverBoundsRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    CoverBoundsRenderStep(Layout, RenderStepID, DepthStencilSettings);
};

}  // namespace skgpu::graphite

#endif // skgpu_render_CoverBoundsRenderStep_DEFINED
