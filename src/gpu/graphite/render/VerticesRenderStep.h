/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_VerticesRenderStep_DEFINED
#define skgpu_graphite_render_VerticesRenderStep_DEFINED

#include "src/base/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <cstdint>
#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
enum class PrimitiveType : uint8_t;

class VerticesRenderStep final : public RenderStep {
public:
    explicit VerticesRenderStep(PrimitiveType, bool hasColor, bool hasTexCoords);

    ~VerticesRenderStep() override;

    std::string vertexSkSL() const override;
    void writeVertices(DrawWriter* writer,
                       const DrawParams& params,
                       skvx::uint2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
    const char* fragmentColorSkSL() const override;

private:
    const bool fHasColor;
    const bool fHasTexCoords;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_VerticesRenderStep_DEFINED
