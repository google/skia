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
                                       GrMtlBuffer* geometryUniformBuffer,
                                       GrMtlBuffer* fragmentUniformBuffer)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat)
        , fGeometryUniformBuffer(geometryUniformBuffer)
        , fFragmentUniformBuffer(fragmentUniformBuffer) {
    (void) fGpu; // Suppress unused-var warning.
    (void) fPixelFormat; // Suppress unused-var warning.
}
