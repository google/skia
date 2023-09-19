/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_ContextFactory_DEFINED
#define skiatest_graphite_ContextFactory_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkTArray.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/GraphiteTestContext.h"

namespace skgpu::graphite {
class Context;
}

namespace skiatest::graphite {

struct ContextInfo {
    GraphiteTestContext* fTestContext = nullptr;
    skgpu::graphite::Context* fContext = nullptr;
};

class ContextFactory {
public:
    explicit ContextFactory(const skgpu::graphite::ContextOptions&);
    ContextFactory() = default;
    ContextFactory(const ContextFactory&) = delete;
    ContextFactory& operator=(const ContextFactory&) = delete;

    ~ContextFactory() = default;

    ContextInfo getContextInfo(skgpu::ContextType);

private:
    struct OwnedContextInfo {
        // This holds the same data as ContextInfo, but uses unique_ptr to maintain ownership.
        skgpu::ContextType fType = skgpu::ContextType::kMock;
        std::unique_ptr<GraphiteTestContext> fTestContext;
        std::unique_ptr<skgpu::graphite::Context> fContext;
    };

    static ContextInfo AsContextInfo(const OwnedContextInfo& ctx);

    skia_private::TArray<OwnedContextInfo> fContexts;
    const skgpu::graphite::ContextOptions fOptions;
};

}  // namespace skiatest::graphite

#endif  // skiatest_graphite_ContextFactory_DEFINED
