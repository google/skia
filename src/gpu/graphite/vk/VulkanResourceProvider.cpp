/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner)
        : ResourceProvider(sharedContext, singleOwner) {}

VulkanResourceProvider::~VulkanResourceProvider() {}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture&) {
    return nullptr;
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary*,
        const GraphicsPipelineDesc&,
        const RenderPassDesc&) {
    return nullptr;
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize, const TextureInfo&, skgpu::Budgeted) {
    return nullptr;
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   PrioritizeGpuReads prioritizeGpuReads) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, prioritizeGpuReads);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SkSamplingOptions&,
                                                     SkTileMode xTileMode,
                                                     SkTileMode yTileMode) {
    return nullptr;
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo&) {
    return {};
}

} // namespace skgpu::graphite
