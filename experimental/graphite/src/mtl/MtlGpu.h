/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlGpu_DEFINED
#define skgpu_MtlGpu_DEFINED

#include "experimental/graphite/src/Gpu.h"

#include "include/ports/SkCFObject.h"

#include "experimental/graphite/include/mtl/MtlBackendContext.h"
#include "experimental/graphite/src/mtl/MtlCaps.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Gpu final : public skgpu::Gpu {
public:
    static sk_sp<skgpu::Gpu> Make(const BackendContext&);
    ~Gpu() override;

    id<MTLDevice> device() const { return fDevice.get(); }
    id<MTLCommandQueue> queue() const { return fQueue.get(); }

    const Caps& mtlCaps() const { return static_cast<const Caps&>(*this->caps()); }

private:
    Gpu(sk_cfp<id<MTLDevice>>, sk_cfp<id<MTLCommandQueue>>, sk_sp<const Caps>);

    bool onSubmit(sk_sp<skgpu::CommandBuffer>) override;

    sk_cfp<id<MTLDevice>> fDevice;
    sk_cfp<id<MTLCommandQueue>> fQueue;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlGpu_DEFINED
