/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

GraphiteResourceKey build_desc_set_key(const SkSpan<DescriptorData>& requestedDescriptors,
                                       const uint32_t uniqueId) {
    // TODO(nicolettep): Optimize key structure. It is horrendously inefficient but functional.
    // Fow now, have each descriptor type and quantity take up an entire uint32_t, with an
    // additional uint32_t added to include a unique identifier for different descriptor sets that
    // have the same set layout.
    static const int kNum32DataCnt = (kDescriptorTypeCount * 2) + 1;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);

    // Fill out the key with each descriptor type. Initialize each count to 0.
    for (uint8_t j = 0; j < kDescriptorTypeCount; j = j + 2) {
        builder[j] = static_cast<uint32_t>(static_cast<DescriptorType>(j));
        builder[j+1] = 0;
    }
    // Go through and update the counts for requested descriptor types. The span should not contain
    // descriptor types with count values of 0, but check just in case.
    for (auto desc : requestedDescriptors) {
        if (desc.count > 0) {
            builder[static_cast<uint32_t>(desc.type) + 1] = desc.count;
        }
    }
    builder[kNum32DataCnt - 1] = uniqueId;
    builder.finish();
    return key;
}

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID)
        : ResourceProvider(sharedContext, singleOwner, recorderID) {}

VulkanResourceProvider::~VulkanResourceProvider() {}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture&) {
    return nullptr;
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        this->skslCompiler(),
                                        runtimeDict,
                                        pipelineDesc,
                                        renderPassDesc);
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size, const TextureInfo& info,
                                                     skgpu::Budgeted budgeted) {
    return VulkanTexture::Make(this->vulkanSharedContext(), size, info, budgeted);
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SkSamplingOptions& samplingOptions,
                                                     SkTileMode xTileMode,
                                                     SkTileMode yTileMode) {
    return VulkanSampler::Make(this->vulkanSharedContext(), samplingOptions, xTileMode, yTileMode);
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo&) {
    return {};
}

sk_sp<VulkanDescriptorSet> VulkanResourceProvider::findOrCreateDescriptorSet(
        SkSpan<DescriptorData> requestedDescriptors) {
    if (requestedDescriptors.empty()) {
        return nullptr;
    }
    // Search for available descriptor sets by assembling a key based upon the set's structure with
    // a unique set ID (which ranges from 0 to kMaxNumSets - 1). Start the search at 0 and continue
    // until an available set is found.
    // TODO(nicolettep): Explore ways to optimize this traversal.
    GraphiteResourceKey descSetKeys [VulkanDescriptorPool::kMaxNumSets];
    for (uint32_t i = 0; i < VulkanDescriptorPool::kMaxNumSets; i++) {
        GraphiteResourceKey key = build_desc_set_key(requestedDescriptors, i);
        if (auto descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kNo)) {
            // A non-null resource pointer indicates we have found an available descriptor set.
            return sk_sp<VulkanDescriptorSet>(static_cast<VulkanDescriptorSet*>(descSet));
        }
        descSetKeys[i] = key;
    }

    // If we did not find an existing avilable desc set, allocate sets with the appropriate layout
    // and add them to the cache.
    VkDescriptorSetLayout layout;
    DescriptorDataToVkDescSetLayout(this->vulkanSharedContext(), requestedDescriptors, &layout);
    if (!layout) {
        return nullptr;
    }
    auto pool = VulkanDescriptorPool::Make(this->vulkanSharedContext(),
                                           requestedDescriptors,
                                           layout);
    SkASSERT(pool);

    // Allocate the maximum number of sets so they can be easily accessed as needed from the cache.
    for (int i = 0; i < VulkanDescriptorPool::kMaxNumSets ; i++) {
        auto descSet = VulkanDescriptorSet::Make(this->vulkanSharedContext(), pool, layout);
        SkASSERT(descSet);
        descSet->setKey(descSetKeys[i]);
        fResourceCache->insertResource(descSet.get());
    }
    auto descSet = fResourceCache->findAndRefResource(descSetKeys[0], skgpu::Budgeted::kNo);
    return descSet ? sk_sp<VulkanDescriptorSet>(static_cast<VulkanDescriptorSet*>(descSet))
                   : nullptr;
}
} // namespace skgpu::graphite
