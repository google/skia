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
#include "include/gpu/graphite/GraphiteTypes.h"
#include "tools/graphite/GraphiteTestContext.h"

// TODO: This is only included to get access to GrContextFactory::ContextType. We should instead
// move all of tools/gpu/ into tools/gpu/ganesh and tools/graphite into tools/gpu/graphite. Then in
// tools gpu we can have files for shared things between ganesh and graphite like ContextType.
#include "tools/gpu/GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

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

        GrContextFactory::ContextType type() const { return fType; }

        skgpu::graphite::Context* context() const { return fContext.get(); }
        GraphiteTestContext* testContext() const { return fTestContext.get(); }

    private:
        friend class ContextFactory; // for ctor

        ContextInfo(GrContextFactory::ContextType type,
                    std::unique_ptr<GraphiteTestContext> testContext,
                    std::unique_ptr<skgpu::graphite::Context> context);

        GrContextFactory::ContextType             fType = GrContextFactory::kMock_ContextType;
        std::unique_ptr<GraphiteTestContext>      fTestContext;
        std::unique_ptr<skgpu::graphite::Context> fContext;
    };

    ContextFactory() = default;
    ContextFactory(const ContextFactory&) = delete;
    ContextFactory& operator=(const ContextFactory&) = delete;

    ~ContextFactory() = default;

    std::tuple<GraphiteTestContext*, skgpu::graphite::Context*> getContextInfo(
            GrContextFactory::ContextType);

private:
    std::vector<ContextInfo> fContexts;
};

} // namespace skiatest::graphite

#endif // skiatest_graphite_ContextFactory_DEFINED
