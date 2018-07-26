/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlCopyManager_DEFINED
#define GrMtlCopyManager_DEFINED

#include "GrTypes.h"

#import <metal/metal.h>

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

private:
    void createCopyProgramBuffer();

    bool createCopyProgram(id<MTLTexture> dstTex);

    id<MTLRenderPipelineState> fCopyProgramPipelineState;
    id<MTLSamplerState>        fCopyProgramSamplerState;
    id<MTLBuffer>              fCopyProgramVertexAttributeBuffer;

    GrMtlGpu* fGpu;
};

#endif
