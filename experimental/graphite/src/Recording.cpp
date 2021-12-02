/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Recording.h"

#include "experimental/graphite/src/CommandBuffer.h"

namespace skgpu {

Recording::Recording(sk_sp<CommandBuffer> commandBuffer)
        : fCommandBuffer(std::move(commandBuffer)){
}

Recording::~Recording() {}

} // namespace skgpu
