/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGpu.h"

#include "experimental/graphite/src/Caps.h"

namespace skgpu::mtl {

sk_sp<skgpu::Gpu> Gpu::Make(const GrMtlBackendContext& context) {
    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(context.fDevice.get()));
    sk_cfp<id<MTLCommandQueue>> queue = sk_ret_cfp((id<MTLCommandQueue>)(context.fQueue.get()));

    return sk_sp<skgpu::Gpu>(new Gpu(std::move(device), std::move(queue)));
}

Gpu::Gpu(sk_cfp<id<MTLDevice>> device, sk_cfp<id<MTLCommandQueue>> queue)
    : fDevice(std::move(device))
    , fQueue(std::move(queue)) {
}

Gpu::~Gpu() {
}

} // namespace skgpu::mtl
