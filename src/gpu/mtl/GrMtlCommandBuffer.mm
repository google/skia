/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlCommandBuffer.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlOpsRenderPass.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"

sk_sp<GrMtlCommandBuffer> GrMtlCommandBuffer::Make(id<MTLCommandQueue> queue) {
    sk_cf_obj<id<MTLCommandBuffer>> mtlCommandBuffer(sk_ref_cf_obj([queue commandBuffer]));
    if (nil == mtlCommandBuffer) {
        return nullptr;
    }

    [*mtlCommandBuffer setLabel: @"GrMtlCommandBuffer::Create"];

    return sk_sp<GrMtlCommandBuffer>(new GrMtlCommandBuffer(mtlCommandBuffer));
}

GrMtlCommandBuffer::~GrMtlCommandBuffer() {
    this->endAllEncoding();
    fTrackedGrBuffers.reset();
    this->callFinishedCallbacks();
}

id<MTLBlitCommandEncoder> GrMtlCommandBuffer::getBlitCommandEncoder() {
    if (fActiveRenderCommandEncoder) {
        [*fActiveRenderCommandEncoder endEncoding];
        fActiveRenderCommandEncoder = nil;
    }

    if (!fActiveBlitCommandEncoder) {
        fActiveBlitCommandEncoder.retain([*fCmdBuffer blitCommandEncoder]);
    }
    fPreviousRenderPassDescriptor = nil;
    fHasWork = true;

    return fActiveBlitCommandEncoder.get();
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
    bool secondDoesntSampleFirst = (!pipelineState ||
                                    pipelineState->doesntSampleAttachment(first)) &&
                                   second.storeAction != MTLStoreActionMultisampleResolve;

    return renderTargetsMatch &&
           (nil == first.texture ||
            (storeActionsValid && loadActionsValid && secondDoesntSampleFirst));
}

id<MTLRenderCommandEncoder> GrMtlCommandBuffer::getRenderCommandEncoder(
        MTLRenderPassDescriptor* descriptor, const GrMtlPipelineState* pipelineState,
        GrMtlOpsRenderPass* opsRenderPass) {
    if (nil != fPreviousRenderPassDescriptor) {
        if (compatible((*fPreviousRenderPassDescriptor).colorAttachments[0],
                       descriptor.colorAttachments[0], pipelineState) &&
            compatible((*fPreviousRenderPassDescriptor).stencilAttachment,
                       descriptor.stencilAttachment, pipelineState)) {
            return fActiveRenderCommandEncoder.get();
        }
    }

    this->endAllEncoding();
    fActiveRenderCommandEncoder.retain(
            [*fCmdBuffer renderCommandEncoderWithDescriptor:descriptor]);
    if (opsRenderPass) {
        opsRenderPass->initRenderState(*fActiveRenderCommandEncoder);
    }
    fPreviousRenderPassDescriptor.retain(descriptor);
    fHasWork = true;

    return fActiveRenderCommandEncoder.get();
}

bool GrMtlCommandBuffer::commit(bool waitUntilCompleted) {
    this->endAllEncoding();
    [*fCmdBuffer commit];
    if (waitUntilCompleted) {
        this->waitUntilCompleted();
    }

    if ((*fCmdBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCmdBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCmdBuffer).status != MTLCommandBufferStatusError);
}

void GrMtlCommandBuffer::endAllEncoding() {
    if (fActiveRenderCommandEncoder) {
        [*fActiveRenderCommandEncoder endEncoding];
        fActiveRenderCommandEncoder.reset();
        fPreviousRenderPassDescriptor.reset();
    }
    if (fActiveBlitCommandEncoder) {
        [*fActiveBlitCommandEncoder endEncoding];
        fActiveBlitCommandEncoder.reset();
    }
}

void GrMtlCommandBuffer::encodeSignalEvent(id<MTLEvent> event, uint64_t eventValue) {
    SkASSERT(fCmdBuffer);
    this->endAllEncoding(); // ensure we don't have any active command encoders
    if (@available(macOS 10.14, iOS 12.0, *)) {
        [*fCmdBuffer encodeSignalEvent:event value:eventValue];
    }
    fHasWork = true;
}

void GrMtlCommandBuffer::encodeWaitForEvent(id<MTLEvent> event, uint64_t eventValue) {
    SkASSERT(fCmdBuffer);
    this->endAllEncoding(); // ensure we don't have any active command encoders
                            // TODO: not sure if needed but probably
    if (@available(macOS 10.14, iOS 12.0, *)) {
        [*fCmdBuffer encodeWaitForEvent:event value:eventValue];
    }
    fHasWork = true;
}

