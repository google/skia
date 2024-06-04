/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_SDFTextLCDRenderStep_DEFINED
#define skgpu_graphite_render_SDFTextLCDRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {

class SDFTextLCDRenderStep final : public RenderStep {
public:
    SDFTextLCDRenderStep();

    ~SDFTextLCDRenderStep() override;

    std::string vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&, skvx::ushort2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_SDFTextLCDRenderStep_DEFINED
