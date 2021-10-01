/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCommandBuffer_DEFINED
#define skgpu_MtlCommandBuffer_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"

#include <memory>

#include "include/core/SkTypes.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    static std::unique_ptr<CommandBuffer> Make(id<MTLCommandQueue>);
    ~CommandBuffer() override {}

private:
    CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer)
        : fCommandBuffer(std::move(cmdBuffer)) {}

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
