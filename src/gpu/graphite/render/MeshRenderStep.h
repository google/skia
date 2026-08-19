/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_MeshRenderStep_DEFINED
#define skgpu_graphite_render_MeshRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderInfo.h"

#include <cstdint>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
struct RootNodesInfo;
class PipelineDataGatherer;

class MeshRenderStep final : public RenderStep {
public:
    explicit MeshRenderStep(Layout);
    ~MeshRenderStep() override;

    std::string vertexSkSL(const RootNodesInfo&) const override;
    std::string fragmentColorSkSL(const RootNodesInfo&) const override;

    void writeVertices(DrawWriter*,
                       StorageContext*,
                       const DrawParams&,
                       uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_MeshRenderStep_DEFINED

