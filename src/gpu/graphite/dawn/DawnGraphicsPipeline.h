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
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"

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
    inline static constexpr unsigned int kGradientBufferIndex = 3;
    inline static constexpr unsigned int kNumUniformBuffers = 4;

    inline static constexpr unsigned int kIntrinsicUniformSize = 32;

    inline static constexpr unsigned int kStaticDataBufferIndex = 0;
    inline static constexpr unsigned int kAppendDataBufferIndex = 1;
    inline static constexpr unsigned int kNumVertexBuffers = 2;

    static sk_sp<DawnGraphicsPipeline> Make(const DawnSharedContext* sharedContext,
                                            DawnResourceProvider* resourceProvider,
                                            const RuntimeEffectDictionary* runtimeDict,
                                            const UniqueKey& pipelineKey,
                                            const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc,
                                            SkEnumBitMask<PipelineCreationFlags>,
                                            uint32_t compilationID);

    ~DawnGraphicsPipeline() override;

    bool didAsyncCompilationFail() const override;

    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    PrimitiveType primitiveType() const { return fPrimitiveType; }

    const wgpu::RenderPipeline& dawnRenderPipeline() const;

    using BindGroupLayouts = std::array<wgpu::BindGroupLayout, kBindGroupCount>;
    const BindGroupLayouts& dawnGroupLayouts() const { return fGroupLayouts; }

    // Returns null if the ith sampler is not an immutable sampler.
    const DawnSampler* immutableSampler(int32_t index) const {
        return fImmutableSamplers[index].get();
    }

private:
    struct AsyncPipelineCreation;

    DawnGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                         const PipelineInfo& pipelineInfo,
                         std::unique_ptr<AsyncPipelineCreation> pipelineCreationInfo,
                         BindGroupLayouts groupLayouts,
                         PrimitiveType primitiveType,
                         uint32_t refValue,
                         skia_private::TArray<sk_sp<DawnSampler>> immutableSamplers);

    void freeGpuData() override;

    std::unique_ptr<AsyncPipelineCreation> fAsyncPipelineCreation;
    BindGroupLayouts fGroupLayouts;
    const PrimitiveType fPrimitiveType;
    const uint32_t fStencilReferenceValue;

    // Hold a ref to immutable samplers used such that their lifetime is properly managed.
    const skia_private::TArray<sk_sp<DawnSampler>> fImmutableSamplers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphicsPipeline_DEFINED
