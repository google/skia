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
        kDirect3D,
        kMetal,
        kVulkan,
        kMock,
    };

    class ContextInfo {
    public:
        ContextInfo() = default;
        ContextInfo(ContextInfo&& other);
        ~ContextInfo() = default;

        ContextFactory::ContextType type() const { return fType; }

        skgpu::Context* context() const { return fContext.get(); }
        GraphiteTestContext* testContext() const { return fTestContext.get(); }

    private:
        friend class ContextFactory; // for ctor

        ContextInfo(ContextFactory::ContextType type,
                    std::unique_ptr<GraphiteTestContext> testContext,
                    std::unique_ptr<skgpu::Context> context);

        ContextType                          fType = ContextType::kMock;
        std::unique_ptr<GraphiteTestContext> fTestContext;
        std::unique_ptr<skgpu::Context>      fContext;
    };

    ContextFactory() = default;
    ContextFactory(const ContextFactory&) = delete;
    ContextFactory& operator=(const ContextFactory&) = delete;

    ~ContextFactory() = default;

    std::tuple<GraphiteTestContext*, skgpu::Context*> getContextInfo(ContextType);

private:
    std::vector<ContextInfo> fContexts;
};

} // namespace skiatest::graphite

#endif // skiatest_graphite_ContextFactory_DEFINED
