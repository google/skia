/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"

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
    fPreviousRenderPassDescriptor = nil;

    return fActiveBlitCommandEncoder;
}

static bool compatible(const MTLRenderPassAttachmentDescriptor* first,
                       const MTLRenderPassAttachmentDescriptor* second,
                       const GrMtlPipelineState* pipelineState) {
    // Check to see if the previous descriptor is compatible with the new one.
    // They are compatible if:
    // * they share the same rendertargets
    // * the first's store actions are either Store or DontCare
    // * the second's load actions are either Load or DontCare
    // * the second doesn't sample from any rendertargets in the first
    bool renderTargetsMatch = (first.texture == second.texture);
    bool storeActionsValid = first.storeAction == MTLStoreActionStore ||
                             first.storeAction == MTLStoreActionDontCare;
    bool loadActionsValid = second.loadAction == MTLLoadActionLoad ||
                            second.loadAction == MTLLoadActionDontCare;
    bool secondDoesntSampleFirst = !pipelineState ||
                                   pipelineState->doesntSampleAttachment(first);

    return renderTargetsMatch &&
           (nil == first.texture ||
            (storeActionsValid && loadActionsValid && secondDoesntSampleFirst));
}

id<MTLRenderCommandEncoder> GrMtlCommandBuffer::getRenderCommandEncoder(
        MTLRenderPassDescriptor* descriptor, const GrMtlPipelineState* pipelineState) {
    if (nil != fPreviousRenderPassDescriptor) {
        if (compatible(fPreviousRenderPassDescriptor.colorAttachments[0],
                       descriptor.colorAttachments[0], pipelineState) &&
            compatible(fPreviousRenderPassDescriptor.stencilAttachment,
                       descriptor.stencilAttachment, pipelineState)) {
            return fActiveRenderCommandEncoder;
        }
    }

    this->endAllEncoding();
    SK_BEGIN_AUTORELEASE_BLOCK
    fActiveRenderCommandEncoder = [fCmdBuffer renderCommandEncoderWithDescriptor:descriptor];
    SK_END_AUTORELEASE_BLOCK
    fPreviousRenderPassDescriptor = descriptor;

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
