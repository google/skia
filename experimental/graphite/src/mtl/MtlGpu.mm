/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGpu.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/mtl/MtlResourceProvider.h"

namespace skgpu::mtl {

sk_sp<skgpu::Gpu> Gpu::Make(const BackendContext& context) {
    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(context.fDevice.get()));
    sk_cfp<id<MTLCommandQueue>> queue = sk_ret_cfp((id<MTLCommandQueue>)(context.fQueue.get()));

    sk_sp<const Caps> caps(new Caps(device.get()));

    return sk_sp<skgpu::Gpu>(new Gpu(std::move(device), std::move(queue), std::move(caps)));
}

Gpu::Gpu(sk_cfp<id<MTLDevice>> device, sk_cfp<id<MTLCommandQueue>> queue, sk_sp<const Caps> caps)
    : skgpu::Gpu(std::move(caps))
    , fDevice(std::move(device))
    , fQueue(std::move(queue)) {
    fResourceProvider.reset(new ResourceProvider(this));
}

Gpu::~Gpu() {
}

} // namespace skgpu::mtl
