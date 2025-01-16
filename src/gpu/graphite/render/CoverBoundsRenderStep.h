/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_CoverBoundsRenderStep_DEFINED
#define skgpu_graphite_render_CoverBoundsRenderStep_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
struct DepthStencilSettings;

class CoverBoundsRenderStep final : public RenderStep {
public:
    CoverBoundsRenderStep(RenderStep::RenderStepID, DepthStencilSettings dsSettings);

    ~CoverBoundsRenderStep() override;

    std::string vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, skvx::uint2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
};

}  // namespace skgpu::graphite

#endif // skgpu_render_CoverBoundsRenderStep_DEFINED
