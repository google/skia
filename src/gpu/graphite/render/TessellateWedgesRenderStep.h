/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class TessellateWedgesRenderStep final : public RenderStep {
public:
    TessellateWedgesRenderStep(std::string_view variantName,
                               DepthStencilSettings depthStencilSettings);

    ~TessellateWedgesRenderStep() override;

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, int ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, SkPipelineDataGatherer*) const override;

};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateWedgesRenderStep_DEFINED
