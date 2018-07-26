/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlCopyManager.h"

#include "GrSurface.h"

#include "GrMtlGpu.h"
#include "GrMtlResourceProvider.h"
#include "GrMtlUtil.h"

#include "SkPoint.h"
#include "SkRect.h"
#include "SkTraceEvent.h"

#import <simd/simd.h>

void GrMtlCopyManager::createCopyProgramBuffer() {
    // Create per vertex attribute data for copy as draw
    static const simd::float2 vdata[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };
    fCopyProgramVertexAttributeBuffer =
            [fGpu->device() newBufferWithLength: sizeof(vdata)
                                        options: MTLResourceStorageModePrivate];
    id<MTLBuffer> transferBuffer = [fGpu->device() newBufferWithBytes: vdata
                                                               length: sizeof(vdata)
                                                              options: MTLResourceStorageModeManaged];

    id<MTLBlitCommandEncoder> blitCmdEncoder = [fGpu->commandBuffer() blitCommandEncoder];
    [blitCmdEncoder copyFromBuffer: transferBuffer
                      sourceOffset: 0
                          toBuffer: fCopyProgramVertexAttributeBuffer
                 destinationOffset: 0
                              size: sizeof(vdata)];
    [blitCmdEncoder endEncoding];
}

bool GrMtlCopyManager::createCopyProgram(id<MTLTexture> dstTex) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    fCopyProgramPipelineState = fGpu->resourceProvider().findOrCreateCopyPipelineState(dstTex);
    if (fCopyProgramPipelineState == nil) {
        return false;
    }

    if (fCopyProgramSamplerState == nil) {
        MTLSamplerDescriptor* samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
        fCopyProgramSamplerState =
                [fGpu->device() newSamplerStateWithDescriptor: samplerDescriptor];
    }
    if (fCopyProgramVertexAttributeBuffer == nil) {
        this->createCopyProgramBuffer();
    }

    SkASSERT(fCopyProgramPipelineState);
    SkASSERT(fCopyProgramSamplerState);
    SkASSERT(fCopyProgramVertexAttributeBuffer);
    return true;
}

bool GrMtlCopyManager::copySurfaceAsDraw(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                         GrSurface* src, GrSurfaceOrigin srcOrigin,
                                         const SkIRect& srcRect, const SkIPoint& dstPoint,
                                         bool canDiscardOutsideDstRect) {
    SkASSERT(fGpu->mtlCaps().canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTarget()),
                                           src->config(), SkToBool(src->asTexture())));

    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst, false);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src, false);

    if (!this->createCopyProgram(dstTex)) {
        return false;
    }

    // UPDATE UNIFORM DESCRIPTOR SET
    int w = srcRect.width();
    int h = srcRect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dstTex.width;
    int dh = dstTex.height;
    float dx0 = 2.f * dstPoint.fX / dw - 1.f;
    float dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    float dy0 = 2.f * dstPoint.fY / dh - 1.f;
    float dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dy0 = -dy0;
        dy1 = -dy1;
    }

    float sx0 = (float)srcRect.fLeft;
    float sx1 = (float)(srcRect.fLeft + w);
    float sy0 = (float)srcRect.fTop;
    float sy1 = (float)(srcRect.fTop + h);
    int sh = srcTex.height;
    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
    // src rect edges in normalized texture space (0 to 1).
    int sw = srcTex.width;
    sx0 /= sw;
    sx1 /= sw;
    sy0 /= sh;
    sy1 /= sh;

    const simd::float4 vertexUniformBuffer[2] = {
        {dx1 - dx0, dy1 - dy0, dx0, dy0}, // posXform
        {sx1 - sx0, sy1 - sy0, sx0, sy0}, // texCoordXform
    };

    MTLRenderPassDescriptor* renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = dstTex;
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    renderPassDesc.colorAttachments[0].loadAction = canDiscardOutsideDstRect ? MTLLoadActionDontCare
                                                                             : MTLLoadActionLoad;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderCmdEncoder =
            [fGpu->commandBuffer() renderCommandEncoderWithDescriptor: renderPassDesc];
    [renderCmdEncoder setRenderPipelineState: fCopyProgramPipelineState];
    [renderCmdEncoder setVertexBuffer: fCopyProgramVertexAttributeBuffer
                               offset: 0
                              atIndex: 0];
    [renderCmdEncoder setVertexBytes: vertexUniformBuffer
                              length: sizeof(vertexUniformBuffer)
                             atIndex: 1];
    [renderCmdEncoder setFragmentTexture: srcTex
                                 atIndex: 0];
    [renderCmdEncoder setFragmentSamplerState: fCopyProgramSamplerState
                                      atIndex: 0];
    [renderCmdEncoder drawPrimitives: MTLPrimitiveTypeTriangleStrip
                         vertexStart: 0
                         vertexCount: 4];
    [renderCmdEncoder endEncoding];
    return true;
}
