/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlPipelineState.h"

#include "GrMtlBuffer.h"
#include "GrMtlGpu.h"

 GrMtlPipelineState::GrMtlPipelineState(GrMtlGpu* gpu,
                                        id<MTLRenderPipelineState> pipelineState,
                                        MTLPixelFormat pixelFormat,
                                        uint32_t geometryUniformSize,
                                        uint32_t fragmentUniformSize)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat) {
    (void) fPixelFormat; // Suppress unused-var warning.
    fGeometryUniformBuffer.reset(GrMtlBuffer::Create(fGpu, geometryUniformSize, kVertex_GrBufferType,
                                                     kStatic_GrAccessPattern));
    fFragmentUniformBuffer.reset(GrMtlBuffer::Create(fGpu, fragmentUniformSize, kVertex_GrBufferType,
                                                     kStatic_GrAccessPattern));
}
