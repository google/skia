/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_DawnTestContext_DEFINED
#define skiatest_graphite_DawnTestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"

#include "include/gpu/graphite/dawn/DawnBackendContext.h"

#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"

namespace skiatest::graphite {

class DawnTestContext : public GraphiteTestContext {
public:
    ~DawnTestContext() override {}

    static std::unique_ptr<GraphiteTestContext> Make();

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kDawn; }

    std::unique_ptr<skgpu::graphite::Context> makeContext() override;

    const skgpu::graphite::DawnBackendContext& getBackendContext() const {
        return fBackendContext;
    }

protected:
    DawnTestContext(const skgpu::graphite::DawnBackendContext& backendContext)
            : fBackendContext(backendContext) {}

    skgpu::graphite::DawnBackendContext     fBackendContext;
};

}  // namespace skiatest::graphite

#endif // skiatest_graphite_DawnTestContext_DEFINED
