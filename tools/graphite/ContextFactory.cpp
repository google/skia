/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/ContextFactory.h"

#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"

#ifdef SK_DAWN
#include "tools/graphite/dawn/GraphiteDawnTestContext.h"
#endif
#ifdef SK_METAL
#include "tools/graphite/mtl/GraphiteMtlTestContext.h"
#endif
#ifdef SK_VULKAN
#include "tools/graphite/vk/GraphiteVulkanTestContext.h"
#endif

namespace skiatest::graphite {

ContextFactory::OwnedContextInfo::OwnedContextInfo() = default;

ContextFactory::OwnedContextInfo::OwnedContextInfo(
        skgpu::ContextType type,
        std::unique_ptr<GraphiteTestContext> testContext,
        std::unique_ptr<skgpu::graphite::Context> context)
        : fType(type), fTestContext(std::move(testContext)), fContext(std::move(context)) {}

ContextFactory::OwnedContextInfo::~OwnedContextInfo() {
    // If we created a non-syncing Context then we have to wait for GPU work to finish before
    // destroying the Context.
    if (fContext && !fContext->priv().caps()->allowCpuSync() && fContext->hasUnfinishedGpuWork()) {
        fTestContext->syncedSubmit(fContext.get());
        SkASSERT(!fContext->hasUnfinishedGpuWork());
    }
}

ContextFactory::OwnedContextInfo::OwnedContextInfo(OwnedContextInfo&&) = default;
ContextFactory::OwnedContextInfo& ContextFactory::OwnedContextInfo::operator=(OwnedContextInfo&&) =
        default;

ContextFactory::ContextFactory(const TestOptions& options)
        : fOptions(options) {}

ContextInfo ContextFactory::AsContextInfo(const OwnedContextInfo& owned) {
    return ContextInfo{owned.fTestContext.get(), owned.fContext.get()};
}

ContextInfo ContextFactory::getContextInfo(skgpu::ContextType type) {
    if (!skgpu::IsDawnBackend(type) && fOptions.fNeverYieldToWebGPU) {
        return {};
    }

    // Look for an existing ContextInfo that we can re-use.
    for (const OwnedContextInfo& ctxInfo : fContexts) {
        if (ctxInfo.fType == type) {
            return AsContextInfo(ctxInfo);
        }
    }

    // Create a new ContextInfo from this context type.
    std::unique_ptr<GraphiteTestContext> testCtx;

    switch (type) {
        case skgpu::ContextType::kMetal: {
#ifdef SK_METAL
            testCtx = graphite::MtlTestContext::Make();
#endif
        } break;
        case skgpu::ContextType::kVulkan: {
#ifdef SK_VULKAN
            testCtx = graphite::VulkanTestContext::Make();
#endif
        } break;
#ifdef SK_DAWN

#define CASE(TYPE)                                                          \
    case skgpu::ContextType::kDawn_##TYPE:                                  \
        testCtx = graphite::DawnTestContext::Make(wgpu::BackendType::TYPE); \
        break;
#else
#define CASE(TYPE)                         \
    case skgpu::ContextType::kDawn_##TYPE: \
        break;
#endif // SK_DAWN
        CASE(D3D11)
        CASE(D3D12)
        CASE(Metal)
        CASE(Vulkan)
        CASE(OpenGL)
        CASE(OpenGLES)
#undef CASE

        default:
            break;
    }

    if (!testCtx) {
        return ContextInfo{};
    }

    std::unique_ptr<skgpu::graphite::Context> context = testCtx->makeContext(fOptions);
    if (!context) {
        return ContextInfo{};
    }

    fContexts.push_back({type, std::move(testCtx), std::move(context)});
    return AsContextInfo(fContexts.back());
}

} // namespace skiatest::graphite
