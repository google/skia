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

#include <optional>

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE
#include "dawn/native/DawnNative.h"

namespace skiatest::graphite {

class DawnTestContext : public GraphiteTestContext {
public:
    ~DawnTestContext() override {}

    static std::unique_ptr<GraphiteTestContext> Make(std::optional<wgpu::BackendType> backend);

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kDawn; }

    skgpu::ContextType contextType() override;

    std::unique_ptr<skgpu::graphite::Context> makeContext(const TestOptions&) override;

    const skgpu::graphite::DawnBackendContext& getBackendContext() const {
        return fBackendContext;
    }

    void tick() override;

protected:
    DawnTestContext(const skgpu::graphite::DawnBackendContext& backendContext)
            : fBackendContext(backendContext) {}

    skgpu::graphite::DawnBackendContext fBackendContext;
};

}  // namespace skiatest::graphite

#endif // skiatest_graphite_DawnTestContext_DEFINED
