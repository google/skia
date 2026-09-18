/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_AnalyticRRectBlurRenderStep_DEFINED
#define skgpu_graphite_render_AnalyticRRectBlurRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
class StaticBufferManager;
struct ResourceBindingRequirements;
struct RootNodesInfo;

class AnalyticRRectBlurRenderStep final : public RenderStep {
public:
    AnalyticRRectBlurRenderStep(Layout, StaticBufferManager*);
    ~AnalyticRRectBlurRenderStep() override = default;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    const char* fragmentCoverageSkSL() const override;

    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    BindBufferInfo fVertexBuffer;
    BindBufferInfo fIndexBuffer;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_render_AnalyticRRectBlurRenderStep_DEFINED
