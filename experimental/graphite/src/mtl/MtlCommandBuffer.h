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
class Gpu;

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    static sk_sp<CommandBuffer> Make(const Gpu*);
    ~CommandBuffer() override {}

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

private:
    CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer)
        : fCommandBuffer(std::move(cmdBuffer)) {}

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
