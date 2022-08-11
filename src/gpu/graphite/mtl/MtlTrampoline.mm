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

std::unique_ptr<QueueManager> MtlTrampoline::MakeQueueManager(const SharedContext* sharedContext) {
    return std::make_unique<MtlQueueManager>(sharedContext);
}

std::unique_ptr<ResourceProvider> MtlTrampoline::MakeResourceProvider(
        const SharedContext* sharedContext,
        sk_sp<GlobalCache> globalCache,
        SingleOwner* singleOwner) {
    return std::make_unique<MtlResourceProvider>(sharedContext,
                                                 std::move(globalCache),
                                                 singleOwner);
}

}  // namespace skgpu::graphite
