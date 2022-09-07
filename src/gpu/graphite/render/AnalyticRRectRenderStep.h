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

class AnalyticRRectRenderStep final : public RenderStep {
public:
    AnalyticRRectRenderStep();

    ~AnalyticRRectRenderStep() override;

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, int ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, SkPipelineDataGatherer*) const override;

private:
};

}  // namespace skgpu::graphite

#endif // skgpu_render_AnalyticRRectRenderStep_DEFINED
