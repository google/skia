/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"

GrMtlCommandBuffer* GrMtlCommandBuffer::Create(id<MTLCommandQueue> queue) {
    id<MTLCommandBuffer> mtlCommandBuffer;
    SK_BEGIN_AUTORELEASE_BLOCK
    mtlCommandBuffer = [queue commandBuffer];
    SK_END_AUTORELEASE_BLOCK

    if (nil == mtlCommandBuffer) {
        return nullptr;
    }

    return new GrMtlCommandBuffer(mtlCommandBuffer);
}

GrMtlCommandBuffer::~GrMtlCommandBuffer() {
    this->endAllEncoding();
    fCmdBuffer = nil;
}

id<MTLBlitCommandEncoder> GrMtlCommandBuffer::getBlitCommandEncoder() {
    if (nil != fActiveRenderCommandEncoder) {
        [fActiveRenderCommandEncoder endEncoding];
        fActiveRenderCommandEncoder = nil;
    }

    if (nil == fActiveBlitCommandEncoder) {
        SK_BEGIN_AUTORELEASE_BLOCK
        fActiveBlitCommandEncoder = [fCmdBuffer blitCommandEncoder];
        SK_END_AUTORELEASE_BLOCK
    }

    return fActiveBlitCommandEncoder;
}

id<MTLRenderCommandEncoder> GrMtlCommandBuffer::getRenderCommandEncoder(
        MTLRenderPassDescriptor* descriptor) {
    // TODO: track whether a compatible renderCommandEncoder is currently active
    this->endAllEncoding();
    SK_BEGIN_AUTORELEASE_BLOCK
    fActiveRenderCommandEncoder = [fCmdBuffer renderCommandEncoderWithDescriptor:descriptor];
    SK_END_AUTORELEASE_BLOCK

    return fActiveRenderCommandEncoder;
}

void GrMtlCommandBuffer::commit(bool waitUntilCompleted) {
    this->endAllEncoding();
    [fCmdBuffer commit];
    if (waitUntilCompleted) {
        [fCmdBuffer waitUntilCompleted];
    }

    if (MTLCommandBufferStatusError == fCmdBuffer.status) {
        NSString* description = fCmdBuffer.error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    fCmdBuffer = nil;
}

void GrMtlCommandBuffer::endAllEncoding() {
    if (nil != fActiveRenderCommandEncoder) {
        [fActiveRenderCommandEncoder endEncoding];
        fActiveRenderCommandEncoder = nil;
    }
    if (nil != fActiveBlitCommandEncoder) {
        [fActiveBlitCommandEncoder endEncoding];
        fActiveBlitCommandEncoder = nil;
    }
}
