/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TextSDFRenderStep_DEFINED
#define skgpu_graphite_render_TextSDFRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu { enum class MaskFormat; }

namespace skgpu::graphite {

class TextSDFRenderStep final : public RenderStep {
public:
    TextSDFRenderStep(bool isA8);

    ~TextSDFRenderStep() override;

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&) const override;
    void writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TextSDFRenderStep_DEFINED
