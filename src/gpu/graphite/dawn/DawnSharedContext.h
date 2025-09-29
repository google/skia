/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnSharedContext_DEFINED
#define skgpu_graphite_DawnSharedContext_DEFINED

#include "src/gpu/graphite/SharedContext.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"

namespace skgpu::graphite {

struct ContextOptions;
struct DawnBackendContext;

class DawnThreadSafeResourceProvider final : public ThreadSafeResourceProvider {
public:
    DawnThreadSafeResourceProvider(std::unique_ptr<ResourceProvider>);
};

class DawnSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const DawnBackendContext&, const ContextOptions&);
    ~DawnSharedContext() override;

    DawnThreadSafeResourceProvider* threadSafeResourceProvider() const;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    const DawnCaps* dawnCaps() const { return static_cast<const DawnCaps*>(this->caps()); }
    const wgpu::Device& device() const { return fDevice; }
    const wgpu::Queue& queue() const { return fQueue; }
    const wgpu::Instance& instance() const { return fInstance; }
    const wgpu::ShaderModule& noopFragment() const { return fNoopFragment; }

    bool hasTick() const { return fTick; }

    void tick() const {
        SkASSERT(this->hasTick());
        fTick(fInstance);
    }

    void deviceTick(Context*) override;

    const wgpu::BindGroupLayout& getUniformBuffersBindGroupLayout() const {
        return fUniformBuffersBindGroupLayout;
    }
    const wgpu::BindGroupLayout& getSingleTextureSamplerBindGroupLayout() const {
        return fSingleTextureSamplerBindGroupLayout;
    }

private:
    DawnSharedContext(const DawnBackendContext&,
                      std::unique_ptr<const DawnCaps>,
                      wgpu::ShaderModule noopFragment,
                      SkExecutor*,
                      SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    void createUniformBuffersBindGroupLayout();
    void createSingleTextureSamplerBindGroupLayout();

    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const UniqueKey&,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&,
                                                   SkEnumBitMask<PipelineCreationFlags>,
                                                   uint32_t compilationID) override;

    wgpu::Instance     fInstance;
    wgpu::Device       fDevice;
    wgpu::Queue        fQueue;
    DawnTickFunction*  fTick;
    // A noop fragment shader, it is used to workaround dawn a validation error(dawn doesn't allow
    // a pipeline with a color attachment but without a fragment shader).
    wgpu::ShaderModule fNoopFragment;

    wgpu::BindGroupLayout fUniformBuffersBindGroupLayout;
    wgpu::BindGroupLayout fSingleTextureSamplerBindGroupLayout;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnSharedContext_DEFINED
