/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_EndCapRenderStep_DEFINED
#define skgpu_graphite_render_EndCapRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;

class EndCapRenderStep final : public RenderStep {
public:
    EndCapRenderStep(Layout);

    ~EndCapRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;
    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_render_EndCapRenderStep_DEFINED
