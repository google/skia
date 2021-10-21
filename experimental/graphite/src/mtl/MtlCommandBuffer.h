/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCommandBuffer_DEFINED
#define skgpu_MtlCommandBuffer_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GpuWorkSubmission.h"

#include <memory>

#include "include/core/SkTypes.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {
class BlitCommandEncoder;
class Gpu;
class RenderCommandEncoder;

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    static sk_sp<CommandBuffer> Make(const Gpu*);
    ~CommandBuffer() override;

    bool isFinished() {
        return (*fCommandBuffer).status == MTLCommandBufferStatusCompleted ||
               (*fCommandBuffer).status == MTLCommandBufferStatusError;

    }
    void waitUntilFinished() {
        // TODO: it's not clear what do to if status is Enqueued. Commit and then wait?
        if ((*fCommandBuffer).status == MTLCommandBufferStatusCommitted) {
            [(*fCommandBuffer) waitUntilCompleted];
        }
    }
    bool commit();

    void copyTextureToBuffer(sk_sp<skgpu::Texture>,
                             SkIRect srcRect,
                             sk_sp<skgpu::Buffer>,
                             size_t bufferOffset,
                             size_t bufferRowBytes) override;

private:
    CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer, const Gpu* gpu);

    void beginRenderPass(const RenderPassDesc&) override;
    void endRenderPass() override;

    void onSetRenderPipeline(sk_sp<skgpu::RenderPipeline>&) override;

    void onDraw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) override;

    BlitCommandEncoder* getBlitCommandEncoder();

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
    sk_sp<RenderCommandEncoder> fActiveRenderCommandEncoder;
    sk_sp<BlitCommandEncoder> fActiveBlitCommandEncoder;

    const Gpu* fGpu;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
