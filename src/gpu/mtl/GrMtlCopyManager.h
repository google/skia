/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlCopyManager_DEFINED
#define GrMtlCopyManager_DEFINED

#include "include/gpu/GrTypes.h"

#import <metal/metal.h>

class GrMtlCopyPipelineState;
class GrMtlGpu;
class GrSurface;
struct SkIPoint;
struct SkIRect;

class GrMtlCopyManager {
public:
    GrMtlCopyManager(GrMtlGpu* gpu) : fGpu(gpu) {}

    bool copySurfaceAsDraw(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                           const SkIRect& srcRect, const SkIPoint& dstPoint,
                           bool canDiscardOutsideDstRect);

    static bool IsCompatible(const GrMtlCopyPipelineState*, MTLPixelFormat dstPixelFormat);

private:
    enum BufferIndex {
        kUniform_BufferIndex,
        kAttribute_BufferIndex,
    };

    void createCopyProgramBuffer();
    void createCopyProgramShaders();
    void createCopyProgramVertexDescriptor();

    void createCopyProgram();

    id<MTLSamplerState>  fSamplerState;
    id<MTLBuffer>        fVertexAttributeBuffer;
    id<MTLFunction>      fVertexFunction;
    id<MTLFunction>      fFragmentFunction;
    MTLVertexDescriptor* fVertexDescriptor;

    GrMtlGpu* fGpu;
};

#endif
