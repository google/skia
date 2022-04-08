/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/PipelineDataCache.h"

namespace skgpu::graphite {

Recording::Recording(sk_sp<CommandBuffer> commandBuffer,
                     std::unique_ptr<TextureDataCache> textureDataCache)
        : fCommandBuffer(std::move(commandBuffer))
        , fTextureDataCache(std::move(textureDataCache)) {
}

Recording::~Recording() {}

} // namespace skgpu::graphite
