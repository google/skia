/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlSharedContext_DEFINED
#define skgpu_graphite_MtlSharedContext_DEFINED

#include "src/gpu/graphite/SharedContext.h"

#include "include/ports/SkCFObject.h"

#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"

#import <Metal/Metal.h>

namespace skgpu {
class MtlMemoryAllocator;
}

namespace skgpu::graphite {
struct ContextOptions;
class MtlGraphicsPipeline;

class MtlThreadSafeResourceProvider final : public ThreadSafeResourceProvider {
public:
    MtlThreadSafeResourceProvider(std::unique_ptr<ResourceProvider>);
};

class MtlSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const MtlBackendContext&, const ContextOptions&);
    ~MtlSharedContext() override;

    skgpu::MtlMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    id<MTLDevice> device() const { return fDevice.get(); }

    const MtlCaps& mtlCaps() const { return static_cast<const MtlCaps&>(*this->caps()); }

    MtlThreadSafeResourceProvider* threadSafeResourceProvider() const;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    sk_cfp<id<MTLDepthStencilState>> getCompatibleDepthStencilState(
        const DepthStencilSettings&) const;


private:

    MtlSharedContext(sk_cfp<id<MTLDevice>>,
                     sk_sp<skgpu::MtlMemoryAllocator>,
                     std::unique_ptr<const MtlCaps>,
                     SkExecutor*,
                     SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    void createCompatibleDepthStencilState(const DepthStencilSettings&);

    sk_sp<GraphicsPipeline> createGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey&,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>,
            uint32_t compilationID) override;

    sk_sp<skgpu::MtlMemoryAllocator> fMemoryAllocator;

    sk_cfp<id<MTLDevice>> fDevice;

    // In the current Graphite class structure 'fDepthStencilStates' would more appropriately
    // go in a new MtlGlobalCache class. However, GlobalCache may go away as a concept, in
    // which case, this would be a reasonable place for this cache.
    // TODO(robertphillips): Come up with a scheme to map from DepthStencilSettings to tightly
    // packed ints and switch this to be a std::array.
    skia_private::THashMap<DepthStencilSettings, sk_cfp<id<MTLDepthStencilState>>>
            fDepthStencilStates;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlSharedContext_DEFINED
