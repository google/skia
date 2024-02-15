/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/ganesh/mtl/GrMtlOpsRenderPass.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineState.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderCommandEncoder.h"
#include "src/gpu/ganesh/mtl/GrMtlSemaphore.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

sk_sp<GrMtlCommandBuffer> GrMtlCommandBuffer::Make(id<MTLCommandQueue> queue) {
    id<MTLCommandBuffer> mtlCommandBuffer;
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        MTLCommandBufferDescriptor* desc = [[MTLCommandBufferDescriptor alloc] init];
        desc.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
        mtlCommandBuffer = [queue commandBufferWithDescriptor:desc];
    } else {
        mtlCommandBuffer = [queue commandBuffer];
    }
#else
    mtlCommandBuffer = [queue commandBuffer];
#endif
    if (nil == mtlCommandBuffer) {
        return nullptr;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    mtlCommandBuffer.label = @"GrMtlCommandBuffer::Make";
#endif

    return sk_sp<GrMtlCommandBuffer>(new GrMtlCommandBuffer(mtlCommandBuffer));
}

GrMtlCommandBuffer::~GrMtlCommandBuffer() {
    this->endAllEncoding();
    this->releaseResources();
    this->callFinishedCallbacks();

    fCmdBuffer = nil;
}

void GrMtlCommandBuffer::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fTrackedResources.clear();
    fTrackedGrBuffers.clear();
    fTrackedGrSurfaces.clear();
}

id<MTLBlitCommandEncoder> GrMtlCommandBuffer::getBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        return fActiveBlitCommandEncoder;
    }

    this->endAllEncoding();
    if (fCmdBuffer.status != MTLCommandBufferStatusNotEnqueued) {
        NSLog(@"GrMtlCommandBuffer: tried to create MTLBlitCommandEncoder while in invalid state.");
        return nullptr;
    }
    fActiveBlitCommandEncoder = [fCmdBuffer blitCommandEncoder];
    fHasWork = true;

    return fActiveBlitCommandEncoder;
}

static bool compatible(const MTLRenderPassAttachmentDescriptor* first,
                       const MTLRenderPassAttachmentDescriptor* second,
                       const GrMtlPipelineState* pipelineState) {
    // From the Metal Best Practices Guide:
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
                                    pipelineState->doesntSampleAttachment(first));

    // Since we are trying to use the same encoder rather than merging two,
    // we have to check to see if both store actions are mutually compatible.
    bool secondStoreValid = true;
    if (second.storeAction == MTLStoreActionDontCare) {
        secondStoreValid = (first.storeAction == MTLStoreActionDontCare);
        // TODO: if first.storeAction is Store and second.loadAction is Load,
        // we could reset the active RenderCommandEncoder's store action to DontCare
    } else if (second.storeAction == MTLStoreActionStore) {
        if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
            secondStoreValid = (first.storeAction == MTLStoreActionStore ||
                                first.storeAction == MTLStoreActionStoreAndMultisampleResolve);
        } else {
            secondStoreValid = (first.storeAction == MTLStoreActionStore);
        }
        // TODO: if the first store action is DontCare we could reset the active
        // RenderCommandEncoder's store action to Store, but it's not clear if it's worth it.
    } else if (second.storeAction == MTLStoreActionMultisampleResolve) {
        if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
            secondStoreValid = (first.resolveTexture == second.resolveTexture) &&
                               (first.storeAction == MTLStoreActionMultisampleResolve ||
                                first.storeAction == MTLStoreActionStoreAndMultisampleResolve);
        } else {
            secondStoreValid = (first.resolveTexture == second.resolveTexture) &&
                               (first.storeAction == MTLStoreActionMultisampleResolve);
        }
        // When we first check whether store actions are valid we don't consider resolves,
        // so we need to reset that here.
        storeActionsValid = secondStoreValid;
    } else {
        if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
            if (second.storeAction == MTLStoreActionStoreAndMultisampleResolve) {
                secondStoreValid = (first.resolveTexture == second.resolveTexture) &&
                                   (first.storeAction == MTLStoreActionStoreAndMultisampleResolve);
                // TODO: if the first store action is simply MultisampleResolve we could reset
                // the active RenderCommandEncoder's store action to StoreAndMultisampleResolve,
                // but it's not clear if it's worth it.

                // When we first check whether store actions are valid we don't consider resolves,
                // so we need to reset that here.
                storeActionsValid = secondStoreValid;
            }
        }
    }

    return renderTargetsMatch &&
           (nil == first.texture ||
            (storeActionsValid && loadActionsValid && secondDoesntSampleFirst && secondStoreValid));
}

GrMtlRenderCommandEncoder* GrMtlCommandBuffer::getRenderCommandEncoder(
        MTLRenderPassDescriptor* descriptor, const GrMtlPipelineState* pipelineState,
        GrMtlOpsRenderPass* opsRenderPass) {
    if (nil != fPreviousRenderPassDescriptor) {
        if (compatible(fPreviousRenderPassDescriptor.colorAttachments[0],
                       descriptor.colorAttachments[0], pipelineState) &&
            compatible(fPreviousRenderPassDescriptor.stencilAttachment,
                       descriptor.stencilAttachment, pipelineState)) {
            return fActiveRenderCommandEncoder.get();
        }
    }

    return this->getRenderCommandEncoder(descriptor, opsRenderPass);
}

GrMtlRenderCommandEncoder* GrMtlCommandBuffer::getRenderCommandEncoder(
        MTLRenderPassDescriptor* descriptor,
        GrMtlOpsRenderPass* opsRenderPass) {
    this->endAllEncoding();
    if (fCmdBuffer.status != MTLCommandBufferStatusNotEnqueued) {
        NSLog(@"GrMtlCommandBuffer: tried to create MTLRenderCommandEncoder while in bad state.");
        return nullptr;
    }
    fActiveRenderCommandEncoder = GrMtlRenderCommandEncoder::Make(
            [fCmdBuffer renderCommandEncoderWithDescriptor:descriptor]);
    if (opsRenderPass) {
        opsRenderPass->initRenderState(fActiveRenderCommandEncoder.get());
    }
    fPreviousRenderPassDescriptor = descriptor;
    fHasWork = true;

    return fActiveRenderCommandEncoder.get();
}

bool GrMtlCommandBuffer::commit(bool waitUntilCompleted) {
    this->endAllEncoding();
    if ([fCmdBuffer status] != MTLCommandBufferStatusNotEnqueued) {
        NSLog(@"GrMtlCommandBuffer: Tried to commit command buffer while in invalid state.\n");
        return false;
    }
    [fCmdBuffer commit];
    if (waitUntilCompleted) {
        this->waitUntilCompleted();
#if defined(SK_BUILD_FOR_IOS) && defined(SK_METAL_WAIT_UNTIL_SCHEDULED)
    // If iOS goes into the background we need to make sure all command buffers are scheduled first.
    // We don't have a way of detecting background transition so this guarantees it.
    } else {
        [fCmdBuffer waitUntilScheduled];
#endif
    }

    if ([fCmdBuffer status] == MTLCommandBufferStatusError) {
        SkDebugf("Error submitting command buffer.\n");
        if (NSError* error = [fCmdBuffer error]) {
            NSLog(@"%@", error);
        }
    }

    return ([fCmdBuffer status] != MTLCommandBufferStatusError);
}

void GrMtlCommandBuffer::endAllEncoding() {
    if (fActiveRenderCommandEncoder) {
        fActiveRenderCommandEncoder->endEncoding();
        fActiveRenderCommandEncoder.reset();
        fPreviousRenderPassDescriptor = nil;
    }
    if (fActiveBlitCommandEncoder) {
        [fActiveBlitCommandEncoder endEncoding];
        fActiveBlitCommandEncoder = nil;
    }
}

void GrMtlCommandBuffer::encodeSignalEvent(sk_sp<GrMtlEvent> event, uint64_t eventValue) {
    SkASSERT(fCmdBuffer);
    this->endAllEncoding(); // ensure we don't have any active command encoders
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        [fCmdBuffer encodeSignalEvent:event->mtlEvent() value:eventValue];
        this->addResource(std::move(event));
    }
    fHasWork = true;
}

void GrMtlCommandBuffer::encodeWaitForEvent(sk_sp<GrMtlEvent> event, uint64_t eventValue) {
    SkASSERT(fCmdBuffer);
    this->endAllEncoding(); // ensure we don't have any active command encoders
                            // TODO: not sure if needed but probably
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        [fCmdBuffer encodeWaitForEvent:event->mtlEvent() value:eventValue];
        this->addResource(std::move(event));
    }
    fHasWork = true;
}

GR_NORETAIN_END
