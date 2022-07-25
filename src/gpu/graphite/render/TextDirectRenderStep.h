/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TextDirectRenderStep_DEFINED
#define skgpu_graphite_render_TextDirectRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class TextDirectRenderStep final : public RenderStep {
public:
    TextDirectRenderStep();

    ~TextDirectRenderStep() override;

    const char* vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(int startBinding) const override;
    const char* fragmentCoverageSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&) const override;
    void writeUniformsAndTextures(const DrawParams&, SkPipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TextDirectRenderStep_DEFINED
