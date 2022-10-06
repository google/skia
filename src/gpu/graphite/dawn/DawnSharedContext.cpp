/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

namespace skgpu::graphite {

sk_sp<SharedContext> DawnSharedContext::Make(const DawnBackendContext& context,
                                             const ContextOptions& options) {
    std::unique_ptr<const DawnCaps> caps(new DawnCaps());

    return sk_sp<SharedContext>(new DawnSharedContext(context,
                                                      std::move(caps)));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps)
        : skgpu::graphite::SharedContext(std::move(caps), BackendApi::kDawn) {}

DawnSharedContext::~DawnSharedContext() = default;

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this, singleOwner));
}

} // namespace skgpu::graphite

