/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnResourceProvider_DEFINED
#define skgpu_graphite_DawnResourceProvider_DEFINED

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

class DawnGraphicsPipeline;
class DawnSampler;
class DawnSharedContext;
class DawnTexture;
class DawnBuffer;
class DawnCommandBuffer;

class DawnResourceProvider final : public ResourceProvider {
public:
    template <size_t NumEntries>
    using BindGroupKey = FixedSizeKey<2 * NumEntries>;

    static constexpr size_t kNumUniformEntries = 3;

    class BlitWithDrawEncoder {
    public:
        BlitWithDrawEncoder(wgpu::RenderPipeline pipeline,
                            bool srcIsMSAA);

        operator bool() const { return fPipeline != nullptr; }

        void EncodeBlit(const wgpu::Device& device,
                        const wgpu::RenderPassEncoder& encoder,
                        const wgpu::TextureView& srcTextureView,
                        const SkIPoint& srcOffset,
                        const SkIRect& dstBounds);

    private:
        wgpu::RenderPipeline fPipeline;
        const bool fSrcIsMSAA;
    };

    DawnResourceProvider(SharedContext* sharedContext,
                         SingleOwner*,
                         uint32_t recorderID,
                         size_t resourceBudget);
    ~DawnResourceProvider() override;

    sk_sp<DawnTexture> findOrCreateDiscardableMSAALoadTexture(SkISize dimensions,
                                                              const TextureInfo& msaaInfo);

    BlitWithDrawEncoder findOrCreateBlitWithDrawEncoder(const RenderPassDesc& renderPassDesc,
                                                        SampleCount srcSampleCount);

    sk_sp<DawnBuffer> findOrCreateDawnBuffer(size_t size,
                                             BufferType type,
                                             AccessPattern,
                                             std::string_view label);

    wgpu::BindGroup createBindGroup(SkSpan<wgpu::BindGroupEntry>, const wgpu::BindGroupLayout);

    // Find or create a bind group containing the given buffer.
    wgpu::BindGroup findOrCreateSingleUniformBindGroup(const BindBufferInfo&);

    // Find or create a bind group containing the given sampler & texture.
    wgpu::BindGroup findOrCreateSingleTextureSamplerBindGroup(const DawnSampler*,
                                                              const DawnTexture*);

    // Find the cached bind buffer info, or create a new one for the given intrinsic values.
    BindBufferInfo findOrCreateIntrinsicBindBufferInfo(DawnCommandBuffer* cb,
                                                       UniformDataBlock intrinsicValues);

    void releasePendingIntrinsicBuffers();

    // For BindGroupEntries, using this method to get a null buffer pointer rather than simply using
    // nullptr allows for assigning a label (when enabled in Caps) for more clear debugging.
    const wgpu::Buffer& getOrCreateNullBuffer();

private:
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;

    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, std::string_view label) override;
    sk_sp<Buffer> createBuffer(size_t, BufferType, AccessPattern, std::string_view label) override;

    sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&, std::string_view label) override;

    sk_sp<Sampler> createSampler(const SamplerDesc&) override;

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(const BackendTexture&) override;

    DawnSharedContext* dawnSharedContext() const;

    void onFreeGpuResources() override;
    void onPurgeResourcesNotUsedSince(
            StdSteadyClock::time_point purgeTime,
            std::optional<StdSteadyClock::time_point> quitPurgingTime) override;

    skia_private::THashMap<uint32_t, wgpu::RenderPipeline> fBlitWithDrawPipelines;

    wgpu::Buffer fNullBuffer;

    class IntrinsicBuffer;
    class IntrinsicConstantsManager;
    std::unique_ptr<IntrinsicConstantsManager> fIntrinsicConstantsManager;

    SingleOwner* fSingleOwner = nullptr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnResourceProvider_DEFINED
