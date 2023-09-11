/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_ContextFactory_DEFINED
#define skiatest_graphite_ContextFactory_DEFINED

#include <vector>
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/GraphiteTestContext.h"

namespace skgpu::graphite {
    class Context;
}

namespace skiatest::graphite {

class ContextFactory {
public:
    class ContextInfo {
    public:
        ContextInfo() = default;
        ContextInfo(ContextInfo&& other);
        ~ContextInfo() = default;

        skgpu::ContextType type() const { return fType; }

        skgpu::graphite::Context* context() const { return fContext.get(); }
        GraphiteTestContext* testContext() const { return fTestContext.get(); }

    private:
        friend class ContextFactory; // for ctor

        ContextInfo(skgpu::ContextType type,
                    std::unique_ptr<GraphiteTestContext> testContext,
                    std::unique_ptr<skgpu::graphite::Context> context);

        skgpu::ContextType fType = skgpu::ContextType::kMock;
        std::unique_ptr<GraphiteTestContext> fTestContext;
        std::unique_ptr<skgpu::graphite::Context> fContext;
    };

    explicit ContextFactory(const skgpu::graphite::ContextOptions&);
    ContextFactory() = default;
    ContextFactory(const ContextFactory&) = delete;
    ContextFactory& operator=(const ContextFactory&) = delete;

    ~ContextFactory();

    std::tuple<GraphiteTestContext*, skgpu::graphite::Context*> getContextInfo(
            skgpu::ContextType);

private:
    std::vector<ContextInfo> fContexts;
    const skgpu::graphite::ContextOptions fOptions;
};

} // namespace skiatest::graphite

#endif // skiatest_graphite_ContextFactory_DEFINED
