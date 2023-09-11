/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_AtlasShapeRenderStep_DEFINED
#define skgpu_graphite_render_AtlasShapeRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class AtlasShapeRenderStep final : public RenderStep {
public:
    AtlasShapeRenderStep();
    ~AtlasShapeRenderStep() override = default;

    std::string vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;

    float boundsOutset(const Transform& localToDevice, const Rect& bounds) const override;

    void writeVertices(DrawWriter*, const DrawParams&, int ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_render_AtlasShapeRenderStep_DEFINED
