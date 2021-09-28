/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCommandBuffer_DEFINED
#define skgpu_MtlCommandBuffer_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    CommandBuffer();
    ~CommandBuffer() override {}

private:
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
