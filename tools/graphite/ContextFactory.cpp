/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/ContextFactory.h"

#include "include/gpu/graphite/Context.h"

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

ContextFactory::ContextFactory(const skgpu::graphite::ContextOptions& options)
        : fOptions(options) {}

ContextInfo ContextFactory::AsContextInfo(const OwnedContextInfo& owned) {
    return ContextInfo{owned.fTestContext.get(), owned.fContext.get()};
}

ContextInfo ContextFactory::getContextInfo(skgpu::ContextType type) {
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
        case skgpu::ContextType::kDawn:
        {
#ifdef SK_DAWN
            // Pass nullopt for default backend.
            testCtx = graphite::DawnTestContext::Make(std::nullopt);
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
