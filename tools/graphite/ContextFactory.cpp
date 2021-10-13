/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/ContextFactory.h"

#include "experimental/graphite/include/Context.h"

#ifdef SK_METAL
#include "tools/graphite/mtl/GraphiteMtlTestContext.h"
#endif

namespace skiatest::graphite {

ContextFactory::ContextInfo::ContextInfo(ContextInfo&& other)
    : fType(other.fType)
    , fTestContext(std::move(other.fTestContext))
    , fContext(std::move(other.fContext)) {
}

ContextFactory::ContextInfo::ContextInfo(ContextFactory::ContextType type,
                                         std::unique_ptr<GraphiteTestContext> testContext,
                                         sk_sp<skgpu::Context> context)
    : fType(type)
    , fTestContext(std::move(testContext))
    , fContext(std::move(context)) {
}

sk_sp<skgpu::Context> ContextFactory::ContextInfo::refContext() const { return fContext; }

////////////////////////////////////////////////////////////////////////////////////////////////////
std::tuple<GraphiteTestContext*, sk_sp<skgpu::Context>> ContextFactory::getContextInfo(
        ContextType type) {

    for (ContextInfo& c : fContexts) {
        if (c.type() == type) {
            return { c.testContext(), c.refContext() };
        }
    }

    std::unique_ptr<GraphiteTestContext> testCtx;

    switch (type) {
        case ContextType::kMetal: {
#ifdef SK_METAL
            testCtx = mtl::TestContext::Make();
#endif
        } break;

        default:
            break;
    }

    if (!testCtx) {
        return {};
    }

    sk_sp<skgpu::Context> context = testCtx->makeContext();
    if (!context) {
        return {};
    }

    fContexts.push_back({ type, std::move(testCtx), std::move(context) });

    return { fContexts.back().testContext(), fContexts.back().refContext() };
}

} // namespace skiatest::graphite
