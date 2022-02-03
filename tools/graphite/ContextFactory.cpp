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
                                         std::unique_ptr<skgpu::Context> context)
    : fType(type)
    , fTestContext(std::move(testContext))
    , fContext(std::move(context)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::tuple<GraphiteTestContext*, skgpu::Context*> ContextFactory::getContextInfo(
        ContextType type) {

    for (ContextInfo& c : fContexts) {
        if (c.type() == type) {
            return { c.testContext(), c.context() };
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

    std::unique_ptr<skgpu::Context> context = testCtx->makeContext();
    if (!context) {
        return {};
    }

    fContexts.push_back({ type, std::move(testCtx), std::move(context) });

    return { fContexts.back().testContext(), fContexts.back().context() };
}

} // namespace skiatest::graphite
