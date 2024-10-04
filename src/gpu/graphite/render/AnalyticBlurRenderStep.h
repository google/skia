/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_AnalyticBlurRenderStep_DEFINED
#define skgpu_graphite_render_AnalyticBlurRenderStep_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
struct ResourceBindingRequirements;

class AnalyticBlurRenderStep final : public RenderStep {
public:
    AnalyticBlurRenderStep();
    ~AnalyticBlurRenderStep() override = default;

    std::string vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&, skvx::uint2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_render_AnalyticBlurRenderStep_DEFINED
