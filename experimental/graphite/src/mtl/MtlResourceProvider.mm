/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlResourceProvider.h"

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"

namespace skgpu::mtl {

ResourceProvider::ResourceProvider(const skgpu::Gpu* gpu)
    : skgpu::ResourceProvider(gpu) {
}

const Gpu* ResourceProvider::mtlGpu() {
    return static_cast<const Gpu*>(fGpu);
}

std::unique_ptr<skgpu::CommandBuffer> ResourceProvider::createCommandBuffer() {
    return CommandBuffer::Make(this->mtlGpu()->queue());
}

} // namespace skgpu::mtl
