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
namespace {

wgpu::ShaderModule CreateNoopFragment(const wgpu::Device& device) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source =
            "@fragment\n"
            "fn main() {}\n";
    wgpu::ShaderModuleDescriptor smDesc;
    smDesc.nextInChain = &wgslDesc;
    auto fsModule = device.CreateShaderModule(&smDesc);
    return fsModule;
}

}

sk_sp<SharedContext> DawnSharedContext::Make(const DawnBackendContext& backendContext,
                                             const ContextOptions& options) {
    if (!backendContext.fDevice || !backendContext.fQueue) {
        return {};
    }

    auto noopFragment = CreateNoopFragment(backendContext.fDevice);
    if (!noopFragment) {
        return {};
    }

    auto caps = std::make_unique<const DawnCaps>(backendContext.fDevice, options);

    return sk_sp<SharedContext>(new DawnSharedContext(backendContext,
                                                      std::move(caps),
                                                      std::move(noopFragment)));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps,
                                     wgpu::ShaderModule noopFragment)
        : skgpu::graphite::SharedContext(std::move(caps), BackendApi::kDawn)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fNoopFragment(std::move(noopFragment)) {}

DawnSharedContext::~DawnSharedContext() = default;

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this, singleOwner));
}

} // namespace skgpu::graphite

