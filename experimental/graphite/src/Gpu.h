/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Gpu_DEFINED
#define skgpu_Gpu_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class Caps;
class ResourceProvider;

class Gpu : public SkRefCnt {
public:
    ~Gpu() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }
    sk_sp<const Caps> refCaps() const { return fCaps; }

    ResourceProvider* resourceProvider() const { return fResourceProvider.get(); }

protected:
    Gpu(sk_sp<const Caps>);

private:
    sk_sp<const Caps> fCaps;

    std::unique_ptr<ResourceProvider> fResourceProvider;
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
