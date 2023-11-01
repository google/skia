/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnGraphicsPipeline_DEFINED
#define skgpu_graphite_DawnGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

class SkUniform;

namespace SkSL {
    class Compiler;
}
namespace skgpu {
struct BlendInfo;
}

namespace skgpu::graphite {

class Attribute;
class Context;
class GraphicsPipelineDesc;
class DawnResourceProvider;
class DawnSharedContext;
struct DepthStencilSettings;
struct RenderPassDesc;
class RuntimeEffectDictionary;

class DawnGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kUniformBufferBindGroupIndex = 0;
    inline static constexpr unsigned int kTextureBindGroupIndex = 1;
    inline static constexpr unsigned int kBindGroupCount = 2;

    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumVertexBuffers = 2;

    static sk_sp<DawnGraphicsPipeline> Make(const DawnSharedContext* sharedContext,
                                            DawnResourceProvider* resourceProvider,
                                            SkSL::Compiler* compiler,
                                            const RuntimeEffectDictionary* runtimeDict,
                                            const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc);

    ~DawnGraphicsPipeline() override {}

    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    PrimitiveType primitiveType() const { return fPrimitiveType; }
    bool hasStepUniforms() const { return fHasStepUniforms; }
    bool hasPaintUniforms() const { return fHasPaintUniforms; }
    const wgpu::RenderPipeline& dawnRenderPipeline() const;

    using BindGroupLayouts = std::array<wgpu::BindGroupLayout, kBindGroupCount>;
    const BindGroupLayouts& dawnGroupLayouts() const { return fGroupLayouts; }

private:
    using AsyncPipelineCreation = DawnAsyncResult<wgpu::RenderPipeline>;

    DawnGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                         PipelineInfo* pipelineInfo,
                         std::unique_ptr<AsyncPipelineCreation> pipelineCreationInfo,
                         BindGroupLayouts groupLayouts,
                         PrimitiveType primitiveType,
                         uint32_t refValue,
                         bool hasStepUniforms,
                         bool hasPaintUniforms);

    void freeGpuData() override;

    std::unique_ptr<AsyncPipelineCreation> fAsyncPipelineCreation;
    BindGroupLayouts fGroupLayouts;
    const PrimitiveType fPrimitiveType;
    const uint32_t fStencilReferenceValue;
    const bool fHasStepUniforms;
    const bool fHasPaintUniforms;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphicsPipeline_DEFINED
