/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED
#define skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;

class TessellateStrokesRenderStep final : public RenderStep {
public:
    // Renders a filled, shaded stroke
    static std::unique_ptr<TessellateStrokesRenderStep> Regular(Layout, bool infinitySupport);

    // Renders a stroke's geometry into the stencil buffer
    static std::unique_ptr<TessellateStrokesRenderStep> StencilOnly(Layout, bool infinitySupport);

    ~TessellateStrokesRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    TessellateStrokesRenderStep(Layout, RenderStepID, SkEnumBitMask<Flags> xtraFlags,
                                bool infinitySupport, StencilSettings);

    bool fInfinitySupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateStrokesRenderStep_DEFINED
