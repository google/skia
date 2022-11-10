/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnSharedContext_DEFINED
#define skgpu_graphite_DawnSharedContext_DEFINED

#include "webgpu/webgpu_cpp.h"

#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"

namespace skgpu::graphite {

struct DawnBackendContext;
struct ContextOptions;

class DawnSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const DawnBackendContext&, const ContextOptions&);
    ~DawnSharedContext() override;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*) override;

    const DawnCaps* dawnCaps() const;
    const wgpu::Device& device() const { return fDevice; }
    const wgpu::Queue& queue() const { return fQueue; }
private:
    DawnSharedContext(const DawnBackendContext&,
                      std::unique_ptr<const DawnCaps> caps);

    wgpu::Device fDevice;
    wgpu::Queue  fQueue;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnSharedContext_DEFINED

