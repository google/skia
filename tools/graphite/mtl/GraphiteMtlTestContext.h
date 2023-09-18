/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_MtlTestContext_DEFINED
#define skiatest_graphite_MtlTestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"

#include "include/gpu/graphite/mtl/MtlBackendContext.h"

namespace skiatest::graphite {

class MtlTestContext : public GraphiteTestContext {
public:
    ~MtlTestContext() override {}

    static std::unique_ptr<GraphiteTestContext> Make();

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kMetal; }

    skgpu::ContextType contextType() override;

    std::unique_ptr<skgpu::graphite::Context> makeContext(
            const skgpu::graphite::ContextOptions&) override;

    const skgpu::graphite::MtlBackendContext& getBackendContext() const {
        return fMtl;
    }

protected:
    MtlTestContext(const skgpu::graphite::MtlBackendContext& mtl) : fMtl(mtl) {}

    skgpu::graphite::MtlBackendContext fMtl;
};

}  // namespace skiatest::graphite

#endif // skiatest_graphite_MtlTestContext_DEFINED
