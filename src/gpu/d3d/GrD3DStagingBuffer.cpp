/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DStagingBuffer.h"

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"

std::unique_ptr<GrStagingBuffer> GrD3DStagingBuffer::Make(GrGpu* gpu, size_t size,
                                                          GrStagingBuffer::Type type, void* data) {
    GrGpuBufferType bufferType = (type == GrStagingBuffer::Type::kTransfer)
                                        ? GrGpuBufferType::kXferCpuToGpu
                                        : GrGpuBufferType::kVertex;
    GrAccessPattern accessPattern = (type == GrStagingBuffer::Type::kTransfer)
                                        ? kDynamic_GrAccessPattern
                                        : kStatic_GrAccessPattern;
    sk_sp<GrD3DBuffer> buffer = GrD3DBuffer::Make(static_cast<GrD3DGpu*>(gpu), size, bufferType,
                                                  accessPattern);
    return std::unique_ptr<GrStagingBuffer>(new GrD3DStagingBuffer(gpu, size, type, data, buffer));
}
