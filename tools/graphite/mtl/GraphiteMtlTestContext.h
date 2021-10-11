/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_MtlTestContext_DEFINED
#define skiatest_graphite_MtlTestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"

#ifdef SK_METAL

#include "experimental/graphite/include/mtl/MtlBackendContext.h"

namespace skiatest::graphite::mtl {

class TestContext : public GraphiteTestContext {
public:
    ~TestContext() override {}

    static std::unique_ptr<GraphiteTestContext> Make();

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kMetal; }

    sk_sp<skgpu::Context> makeContext() override;

    const skgpu::mtl::BackendContext& getBackendContext() const {
        return fMtl;
    }

protected:
    TestContext(const skgpu::mtl::BackendContext& mtl) : fMtl(mtl) {}

    skgpu::mtl::BackendContext fMtl;
};

}  // namespace skiatest::graphite::mtl

#endif // SK_METAL

#endif // skiatest_graphite_MtlTestContext_DEFINED
