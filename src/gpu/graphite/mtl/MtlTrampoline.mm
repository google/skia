/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTrampoline.h"

#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"

namespace skgpu::graphite {
sk_sp<skgpu::graphite::Gpu> MtlTrampoline::MakeGpu(const MtlBackendContext& backendContext,
                                                   const ContextOptions& options) {
    return MtlGpu::Make(backendContext, options);
}

std::unique_ptr<QueueManager> MtlTrampoline::MakeQueueManager(Gpu* gpu) {
    return std::make_unique<MtlQueueManager>(gpu);
}

std::unique_ptr<ResourceProvider> MtlTrampoline::MakeResourceProvider(
        const Gpu* gpu, sk_sp<GlobalCache> globalCache, SingleOwner* singleOwner) {
    return std::make_unique<MtlResourceProvider>(gpu, std::move(globalCache), singleOwner);
}

}  // namespace skgpu::graphite
