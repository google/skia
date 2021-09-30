/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceProvider.h"

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Pipeline.h"

namespace skgpu {

ResourceProvider::ResourceProvider() {
}

ResourceProvider::~ResourceProvider() {
}

std::unique_ptr<CommandBuffer> ResourceProvider::createCommandBuffer() {
    auto cb = this->onCreateCommandBuffer();

    return cb;
}

Pipeline* ResourceProvider::findOrCreatePipeline(const PipelineDesc& desc) {
    // TODO: look through cache for matching pipeline

    auto pso = this->onCreatePipeline();

    // TODO: cache new pipeline

    return pso;
}

std::unique_ptr<CommandBuffer> onCreateCommandBuffer() {
    return nullptr;
}

} // namespace skgpu
