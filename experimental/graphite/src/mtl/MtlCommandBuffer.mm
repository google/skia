/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlRenderCommandEncoder.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"

namespace skgpu::mtl {

sk_sp<CommandBuffer> CommandBuffer::Make(const Gpu* gpu) {
    sk_cfp<id<MTLCommandBuffer>> cmdBuffer;
    id<MTLCommandQueue> queue = gpu->queue();
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        sk_cfp<MTLCommandBufferDescriptor*> desc([[MTLCommandBufferDescriptor alloc] init]);
        (*desc).retainedReferences = NO;
#ifdef SK_ENABLE_MTL_DEBUG_INFO
        (*desc).errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
#endif
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithDescriptor:desc.get()] retain]);
    } else {
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithUnretainedReferences] retain]);
    }
    if (cmdBuffer == nil) {
        return nullptr;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
     (*cmdBuffer).label = @"CommandBuffer::Make";
#endif

    return sk_sp<CommandBuffer>(new CommandBuffer(std::move(cmdBuffer)));
}

CommandBuffer::CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer)
    : fCommandBuffer(std::move(cmdBuffer)) {}

CommandBuffer::~CommandBuffer() {}

bool CommandBuffer::commit() {
    // TODO: end any encoding
    [(*fCommandBuffer) commit];

    // TODO: better error reporting
    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

void CommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc) {
    SkASSERT(!fActiveRenderCommandEncoder);

    auto& colorInfo = renderPassDesc.fColorAttachment;

    const static MTLLoadAction mtlLoadAction[] {
        MTLLoadActionLoad,
        MTLLoadActionClear,
        MTLLoadActionDontCare
    };
    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(SK_ARRAY_COUNT(mtlLoadAction) == kLoadOpCount);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(SK_ARRAY_COUNT(mtlStoreAction) == kStoreOpCount);

    sk_cfp<MTLRenderPassDescriptor*> descriptor([[MTLRenderPassDescriptor alloc] init]);
    // Set up color attachment.
    auto colorAttachment = (*descriptor).colorAttachments[0];
    Texture* colorTexture = (Texture*)colorInfo.fTexture.get();
    colorAttachment.texture = colorTexture->mtlTexture();
    const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
    colorAttachment.clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
    colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

    // TODO:
    // * setup resolve
    // * set up stencil and depth

    fActiveRenderCommandEncoder = RenderCommandEncoder::Make(fCommandBuffer.get(),
                                                             descriptor.get());

    if (colorTexture) {
        this->trackResource(sk_ref_sp(colorTexture));
    }
    this->trackResource(fActiveRenderCommandEncoder);

    if (colorInfo.fStoreOp == StoreOp::kStore) {
        fHasWork = true;
    }
}

void CommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderCommandEncoder);
    fActiveRenderCommandEncoder->endEncoding();
    fActiveRenderCommandEncoder.reset();
}

} // namespace skgpu::mtl
