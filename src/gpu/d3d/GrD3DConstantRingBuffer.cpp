/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DConstantRingBuffer.h"

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"

sk_sp<GrD3DConstantRingBuffer> GrD3DConstantRingBuffer::Make(GrD3DGpu* gpu, size_t size,
                                                             size_t alignment) {
    sk_sp<GrGpuBuffer> buffer = GrD3DBuffer::Make(gpu, size, GrGpuBufferType::kVertex,
                                                  kDynamic_GrAccessPattern);
    if (!buffer) {
        return nullptr;
    }

    return sk_sp<GrD3DConstantRingBuffer>(new GrD3DConstantRingBuffer(std::move(buffer), size,
                                                                      alignment, gpu));
}

sk_sp<GrGpuBuffer> GrD3DConstantRingBuffer::createBuffer(size_t size) {
    // Make sure the old buffer is added to the current command list
    fGpu->resourceProvider().prepForSubmit();

    return GrD3DBuffer::Make(fGpu, size, GrGpuBufferType::kVertex, kDynamic_GrAccessPattern);
}
