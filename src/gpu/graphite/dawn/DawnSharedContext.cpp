/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {
namespace {

wgpu::ShaderModule CreateNoopFragment(const wgpu::Device& device) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code =
            "@fragment\n"
            "fn main() {}\n";
    wgpu::ShaderModuleDescriptor smDesc;
    smDesc.nextInChain = &wgslDesc;
    smDesc.label = "no-op";
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

    auto caps = std::make_unique<const DawnCaps>(backendContext, options);

    return sk_sp<SharedContext>(new DawnSharedContext(backendContext,
                                                      std::move(caps),
                                                      std::move(noopFragment),
                                                      options.fExecutor,
                                                      options.fUserDefinedKnownRuntimeEffects));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps,
                                     wgpu::ShaderModule noopFragment,
                                     SkExecutor* executor,
                                     SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps),
                        BackendApi::kDawn,
                        executor,
                        userDefinedKnownRuntimeEffects)
        , fInstance(backendContext.fInstance)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fTick(backendContext.fTick)
        , fNoopFragment(std::move(noopFragment)) {}

DawnSharedContext::~DawnSharedContext() {
    // need to clear out resources before any allocator is removed
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this,
                                                                      singleOwner,
                                                                      recorderID,
                                                                      resourceBudget));
}

void DawnSharedContext::deviceTick(Context* context) {
#if !defined(__EMSCRIPTEN__)
    this->device().Tick();
#endif
    context->checkAsyncWorkCompletion();
};

} // namespace skgpu::graphite
