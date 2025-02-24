/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlSharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/mtl/MtlMemoryAllocatorImpl.h"

namespace skgpu::graphite {

sk_sp<SharedContext> MtlSharedContext::Make(const MtlBackendContext& context,
                                            const ContextOptions& options) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // no warning needed
    } else {
        SKGPU_LOG_E("Skia's Graphite backend no longer supports this OS version.");
#ifdef SK_BUILD_FOR_IOS
        SKGPU_LOG_E("Minimum supported version is iOS/tvOS 13.0.");
#else
        SKGPU_LOG_E("Minimum supported version is MacOS 10.15.");
#endif
        return nullptr;
    }

    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(context.fDevice.get()));

    std::unique_ptr<const MtlCaps> caps(new MtlCaps(device.get(), options));

    // TODO: Add memory allocator to context once we figure out synchronization
    sk_sp<MtlMemoryAllocator> memoryAllocator = skgpu::MtlMemoryAllocatorImpl::Make(device.get());
    if (!memoryAllocator) {
        SkDEBUGFAIL("No supplied Metal memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new MtlSharedContext(std::move(device),
                                                     std::move(memoryAllocator),
                                                     std::move(caps),
                                                     options.fUserDefinedKnownRuntimeEffects));
}

MtlSharedContext::MtlSharedContext(sk_cfp<id<MTLDevice>> device,
                                   sk_sp<skgpu::MtlMemoryAllocator> memoryAllocator,
                                   std::unique_ptr<const MtlCaps> caps,
                                   SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps), BackendApi::kMetal, userDefinedKnownRuntimeEffects)
        , fMemoryAllocator(std::move(memoryAllocator))
        , fDevice(std::move(device)) {}

MtlSharedContext::~MtlSharedContext() {
    // need to clear out resources before the allocator (if any) is removed
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> MtlSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new MtlResourceProvider(this,
                                                                     singleOwner,
                                                                     recorderID,
                                                                     resourceBudget));
}

} // namespace skgpu::graphite
