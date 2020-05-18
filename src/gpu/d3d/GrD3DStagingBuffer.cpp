/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DStagingBuffer.h"

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"

std::unique_ptr<GrStagingBuffer> GrD3DStagingBuffer::Make(GrGpu* gpu, size_t size) {
    sk_sp<GrD3DBuffer> buffer = GrD3DBuffer::Make(static_cast<GrD3DGpu*>(gpu), size,
                                                  GrGpuBufferType::kXferCpuToGpu,
                                                  kDynamic_GrAccessPattern);
    // will persistently map this buffer
    void* mapPtr = buffer->map();
    return std::unique_ptr<GrStagingBuffer>(new GrD3DStagingBuffer(gpu, size, mapPtr, buffer));
}
