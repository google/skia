/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTrampoline.h"

#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

sk_sp<SharedContext> MtlTrampoline::MakeSharedContext(const MtlBackendContext& backendContext,
                                                      const ContextOptions& options) {
    return MtlSharedContext::Make(backendContext, options);
}

std::unique_ptr<QueueManager> MtlTrampoline::MakeQueueManager(
        const MtlBackendContext& backendContext, const SharedContext* sharedContext) {

    sk_cfp<id<MTLCommandQueue>> queue =
            sk_ret_cfp((id<MTLCommandQueue>)(backendContext.fQueue.get()));
    return std::make_unique<MtlQueueManager>(std::move(queue), sharedContext);
}

std::unique_ptr<ResourceProvider> MtlTrampoline::MakeResourceProvider(
        SharedContext* sharedContext,
        SingleOwner* singleOwner) {
    return std::make_unique<MtlResourceProvider>(sharedContext, singleOwner);
}

}  // namespace skgpu::graphite
