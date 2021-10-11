/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_ContextFactory_DEFINED
#define skiatest_graphite_ContextFactory_DEFINED

#include <vector>
#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkRefCnt.h"
#include "tools/graphite/GraphiteTestContext.h"

namespace skgpu {
    class Context;
};

namespace skiatest::graphite {

class ContextFactory {
public:
    enum class ContextType {
        kMetal,
        kMock,
    };

    class ContextInfo {
    public:
        ContextInfo() = default;
        ContextInfo(ContextInfo&& other)
           : fType(other.fType)
           , fTestContext(std::move(other.fTestContext))
           , fContext(std::move(other.fContext)) {
        }

        ~ContextInfo() = default;

        ContextFactory::ContextType type() const { return fType; }

        skgpu::Context* context() const { return fContext.get(); }
        sk_sp<skgpu::Context> refContext() const { return fContext; }
        GraphiteTestContext* testContext() const { return fTestContext.get(); }

    private:
        friend class ContextFactory; // for ctor

        ContextInfo(ContextFactory::ContextType type,
                    std::unique_ptr<GraphiteTestContext> testContext,
                    sk_sp<skgpu::Context> context)
            : fType(type)
            , fTestContext(std::move(testContext))
            , fContext(std::move(context)) {
        }

        ContextType                          fType = ContextType::kMock;
        std::unique_ptr<GraphiteTestContext> fTestContext;
        sk_sp<skgpu::Context>                fContext;
    };

    ContextFactory() = default;
    ContextFactory(const ContextFactory&) = delete;
    ContextFactory& operator=(const ContextFactory&) = delete;

    ~ContextFactory() = default;

    std::tuple<GraphiteTestContext*, sk_sp<skgpu::Context>> getContextInfo(ContextType);

private:
    std::vector<ContextInfo> fContexts;
};

} // namespace skiatest::graphite

#endif // skiatest_graphite_ContextFactory_DEFINED
