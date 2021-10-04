/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"

namespace skgpu::mtl {

std::unique_ptr<CommandBuffer> CommandBuffer::Make(id<MTLCommandQueue> queue) {
    sk_cfp<id<MTLCommandBuffer>> cmdBuffer;
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        sk_cfp<MTLCommandBufferDescriptor*> desc([[MTLCommandBufferDescriptor alloc] init]);
        (*desc).retainedReferences = NO;
#ifdef SK_ENABLE_MTL_DEBUG_INFO
        (*desc).errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
#endif
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithDescriptor:desc.get()] retain]);
    } else {
#endif
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithUnretainedReferences] retain]);
#if GR_METAL_SDK_VERSION >= 230
    }
#endif
    if (cmdBuffer == nil) {
        return nullptr;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
     (*cmdBuffer).label = @"CommandBuffer::Make";
#endif

    return std::unique_ptr<CommandBuffer>(new CommandBuffer(std::move(cmdBuffer)));
}

} // namespace skgpu::mtl
