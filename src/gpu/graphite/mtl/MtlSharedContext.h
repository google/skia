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
#include "src/gpu/graphite/mtl/MtlCaps.h"

#import <Metal/Metal.h>

namespace skgpu {
class MtlMemoryAllocator;
}

namespace skgpu::graphite {
struct ContextOptions;

class MtlSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const MtlBackendContext&, const ContextOptions&);
    ~MtlSharedContext() override;

    skgpu::MtlMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    id<MTLDevice> device() const { return fDevice.get(); }

    const MtlCaps& mtlCaps() const { return static_cast<const MtlCaps&>(*this->caps()); }

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

private:

    MtlSharedContext(sk_cfp<id<MTLDevice>>,
                     sk_sp<skgpu::MtlMemoryAllocator> memoryAllocator,
                     std::unique_ptr<const MtlCaps>,
                     SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    sk_sp<skgpu::MtlMemoryAllocator> fMemoryAllocator;

    sk_cfp<id<MTLDevice>> fDevice;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlSharedContext_DEFINED
