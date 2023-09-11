/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_SDFTextRenderStep_DEFINED
#define skgpu_graphite_render_SDFTextRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {

class SDFTextRenderStep final : public RenderStep {
public:
    SDFTextRenderStep(bool isLCD);

    ~SDFTextRenderStep() override;

    std::string vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&, int ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_SDFTextRenderStep_DEFINED
