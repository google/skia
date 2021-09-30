/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTestContext_DEFINED
#define skgpu_MtlTestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"

#ifdef SK_METAL

#include "experimental/graphite/include/mtl/MtlBackendContext.h"

namespace sk_graphite_test::mtl {

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

}  // namespace sk_graphite_test::mtl

#endif // SK_METAL

#endif // skgpu_MtlTestContext_DEFINED
