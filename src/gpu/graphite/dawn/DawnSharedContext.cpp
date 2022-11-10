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
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

namespace skgpu::graphite {

sk_sp<SharedContext> DawnSharedContext::Make(const DawnBackendContext& backendContext,
                                             const ContextOptions& options) {
    if (!backendContext.fDevice || !backendContext.fQueue) {
        return {};
    }

    auto caps = std::make_unique<const DawnCaps>(backendContext.fDevice, options);

    return sk_sp<SharedContext>(new DawnSharedContext(backendContext,
                                                      std::move(caps)));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps)
        : skgpu::graphite::SharedContext(std::move(caps), BackendApi::kDawn)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue) {}

DawnSharedContext::~DawnSharedContext() = default;

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this, singleOwner));
}

const DawnCaps* DawnSharedContext::dawnCaps() const {
    return static_cast<const DawnCaps*>(caps());
}

} // namespace skgpu::graphite

