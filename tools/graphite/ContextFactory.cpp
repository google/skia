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
#include "tools/graphite/vk/VulkanTestContext.h"
#endif

namespace skiatest::graphite {

ContextFactory::ContextInfo::ContextInfo(ContextInfo&& other)
    : fType(other.fType)
    , fTestContext(std::move(other.fTestContext))
    , fContext(std::move(other.fContext)) {
}

ContextFactory::ContextInfo::ContextInfo(GrContextFactory::ContextType type,
                                         std::unique_ptr<GraphiteTestContext> testContext,
                                         std::unique_ptr<skgpu::graphite::Context> context)
    : fType(type)
    , fTestContext(std::move(testContext))
    , fContext(std::move(context)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::tuple<GraphiteTestContext*, skgpu::graphite::Context*> ContextFactory::getContextInfo(
        GrContextFactory::ContextType type) {

    for (ContextInfo& c : fContexts) {
        if (c.type() == type) {
            return { c.testContext(), c.context() };
        }
    }

    std::unique_ptr<GraphiteTestContext> testCtx;

    switch (type) {
        case GrContextFactory::kDawn_ContextType: {
#ifdef SK_DAWN
            testCtx = graphite::DawnTestContext::Make();
#endif
        } break;
        case GrContextFactory::kMetal_ContextType: {
#ifdef SK_METAL
            testCtx = graphite::MtlTestContext::Make();
#endif
        } break;
        case GrContextFactory::kVulkan_ContextType: {
#ifdef SK_VULKAN
            testCtx = graphite::VulkanTestContext::Make();
#endif
        } break;
        default:
            break;
    }

    if (!testCtx) {
        return {};
    }

    std::unique_ptr<skgpu::graphite::Context> context = testCtx->makeContext();
    if (!context) {
        return {};
    }

    fContexts.push_back({ type, std::move(testCtx), std::move(context) });

    return { fContexts.back().testContext(), fContexts.back().context() };
}

} // namespace skiatest::graphite
