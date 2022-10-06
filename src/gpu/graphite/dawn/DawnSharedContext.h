/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnSharedContext_DEFINED
#define skgpu_graphite_DawnSharedContext_DEFINED

#include "src/gpu/graphite/SharedContext.h"


namespace skgpu::graphite {

struct DawnBackendContext;
struct ContextOptions;
class DawnCaps;

class DawnSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const DawnBackendContext&, const ContextOptions&);
    ~DawnSharedContext() override;

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*) override;

private:
    DawnSharedContext(const DawnBackendContext&,
                      std::unique_ptr<const DawnCaps> caps);

};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnSharedContext_DEFINED

