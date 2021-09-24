/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlGpu_DEFINED
#define skgpu_MtlGpu_DEFINED

#include "experimental/graphite/src/Gpu.h"

#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Gpu final : public skgpu::Gpu {
public:
    static sk_sp<skgpu::Gpu> Make(const GrMtlBackendContext&);
    ~Gpu() override;

    id<MTLDevice> device() const { return fDevice.get(); }

private:
    Gpu(sk_cfp<id<MTLDevice>>, sk_cfp<id<MTLCommandQueue>>);

    sk_cfp<id<MTLDevice>> fDevice;
    sk_cfp<id<MTLCommandQueue>> fQueue;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlGpu_DEFINED
