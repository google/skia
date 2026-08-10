/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED

#include "src/core/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;

class TessellateStrokesRenderStep final : public RenderStep {
public:
    explicit TessellateStrokesRenderStep(Layout, bool infinitySupport, bool inverseFill);

    ~TessellateStrokesRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    void writeVertices(DrawWriter*, const DrawParams&, uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    bool fInfinitySupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED
