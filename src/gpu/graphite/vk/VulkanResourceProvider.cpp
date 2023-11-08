/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanFramebuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID,
                                               size_t resourceBudget,
                                               sk_sp<Buffer> intrinsicConstantUniformBuffer)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fIntrinsicUniformBuffer(std::move(intrinsicConstantUniformBuffer)) {
}

VulkanResourceProvider::~VulkanResourceProvider() {
    if (fPipelineCache != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineCache(this->vulkanSharedContext()->device(),
                                         fPipelineCache,
                                         nullptr));
    }
}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() const {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    return VulkanTexture::MakeWrapped(this->vulkanSharedContext(),
                                      this,
                                      texture.dimensions(),
                                      texture.info(),
                                      texture.getMutableState(),
                                      texture.getVkImage(),
                                      /*alloc=*/{}); // Skia does not own wrapped texture memory
}

sk_sp<Buffer> VulkanResourceProvider::refIntrinsicConstantBuffer() const {
    return fIntrinsicUniformBuffer;
}


sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    SkSL::Compiler skslCompiler(fSharedContext->caps()->shaderCaps());
    auto compatibleRenderPass =
            this->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/true);
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        &skslCompiler,
                                        runtimeDict,
                                        pipelineDesc,
                                        renderPassDesc,
                                        compatibleRenderPass,
                                        this->pipelineCache());
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size, const TextureInfo& info,
                                                     skgpu::Budgeted budgeted) {
    return VulkanTexture::Make(this->vulkanSharedContext(), this, size, info, budgeted);
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
                                                              const TextureInfo& info) {
    VulkanTextureInfo vkTexInfo;
    if (!info.getVulkanTextureInfo(&vkTexInfo)) {
        return {};
    }
    VulkanTexture::CreatedImageInfo createdTextureInfo;
    if (!VulkanTexture::MakeVkImage(this->vulkanSharedContext(), dimensions, info,
                                    &createdTextureInfo)) {
        return {};
    }
    return {dimensions,
            vkTexInfo,
            createdTextureInfo.fMutableState->getImageLayout(),
            createdTextureInfo.fMutableState->getQueueFamilyIndex(),
            createdTextureInfo.fImage,
            createdTextureInfo.fMemoryAlloc};
}

namespace {
GraphiteResourceKey build_desc_set_key(const SkSpan<DescriptorData>& requestedDescriptors,
                                       const uint32_t uniqueId) {
    // TODO(nicolettep): Finalize & optimize key structure. Refactor to have the order of the
    // requested descriptors be irrelevant.
    // For now, to place some kind of upper limit on key size, limit a key to only containing
    // information for up to 9 descriptors. This number was selected due to having a maximum of 3
    // uniform buffer descriptors and observationally only encountering up to 6 texture/samplers for
    // our testing use cases. The 10th uint32 is reserved for housing a unique descriptor set ID.
    static const int kMaxDescriptorQuantity = 9;
    static const int kNum32DataCnt = kMaxDescriptorQuantity + 1;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);

    if (requestedDescriptors.size() > kMaxDescriptorQuantity) {
        SKGPU_LOG_E("%d descriptors requested, but graphite currently only supports creating"
                    "descriptor set keys for up to %d. The key will only take the first %d into"
                    " account.", static_cast<int>(requestedDescriptors.size()),
                    kMaxDescriptorQuantity, kMaxDescriptorQuantity);
    }

    for (size_t i = 0; i < kNum32DataCnt; i++) {
        if (i < requestedDescriptors.size()) {
            // TODO: Consider making the DescriptorData struct itself just use uint16_t.
            uint16_t smallerCount = static_cast<uint16_t>(requestedDescriptors[i].count);
            builder[i] =  static_cast<uint8_t>(requestedDescriptors[i].type) << 24
                          | requestedDescriptors[i].bindingIndex << 16
                          | smallerCount;
        } else {
            // Populate reminaing key components with 0.
            builder[i] = 0;
        }
    }
    builder[kNum32DataCnt - 1] = uniqueId;
    builder.finish();
    return key;
}

sk_sp<VulkanDescriptorSet> add_new_desc_set_to_cache(const VulkanSharedContext* context,
                                                     const sk_sp<VulkanDescriptorPool>& pool,
                                                     const GraphiteResourceKey& descSetKey,
                                                     ResourceCache* resourceCache) {
    sk_sp<VulkanDescriptorSet> descSet = VulkanDescriptorSet::Make(context, pool);
    if (!descSet) {
        return nullptr;
    }
    descSet->setKey(descSetKey);
    resourceCache->insertResource(descSet.get());

    return descSet;
}
} // anonymous namespace

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
        if (auto descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kYes)) {
            // A non-null resource pointer indicates we have found an available descriptor set.
            return sk_sp<VulkanDescriptorSet>(static_cast<VulkanDescriptorSet*>(descSet));
        }
        descSetKeys[i] = key;
    }

    // If we did not find an existing avilable desc set, allocate sets with the appropriate layout
    // and add them to the cache.
    VkDescriptorSetLayout layout;
    const VulkanSharedContext* context = this->vulkanSharedContext();
    DescriptorDataToVkDescSetLayout(context, requestedDescriptors, &layout);
    if (!layout) {
        return nullptr;
    }
    auto pool = VulkanDescriptorPool::Make(context, requestedDescriptors, layout);
    if (!pool) {
        VULKAN_CALL(context->interface(), DestroyDescriptorSetLayout(context->device(),
                                                                     layout,
                                                                     nullptr));
        return nullptr;
    }

    // Start with allocating one descriptor set. If one cannot be successfully created, then we can
    // return early before attempting to allocate more. Storing a ptr to the first set also
    // allows us to return that later without having to perform a find operation on the cache once
    // all the sets are added.
    auto firstDescSet =
            add_new_desc_set_to_cache(context, pool, descSetKeys[0], fResourceCache.get());
    if (!firstDescSet) {
        return nullptr;
    }

    // Continue to allocate & cache the maximum number of sets so they can be easily accessed as
    // they're needed.
    for (int i = 1; i < VulkanDescriptorPool::kMaxNumSets ; i++) {
        auto descSet =
                add_new_desc_set_to_cache(context, pool, descSetKeys[i], fResourceCache.get());
        if (!descSet) {
            SKGPU_LOG_W("Descriptor set allocation %d of %d was unsuccessful; no more sets will be"
                        "allocated from this pool.", i, VulkanDescriptorPool::kMaxNumSets);
            break;
        }
    }

    return firstDescSet;
}

sk_sp<VulkanRenderPass> VulkanResourceProvider::findOrCreateRenderPass(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {
    auto renderPassKey = VulkanRenderPass::MakeRenderPassKey(renderPassDesc, compatibleOnly);
    if (Resource* resource =
            fResourceCache->findAndRefResource(renderPassKey, skgpu::Budgeted::kYes)) {
        return sk_sp<VulkanRenderPass>(static_cast<VulkanRenderPass*>(resource));
    }

    auto renderPass = VulkanRenderPass::MakeRenderPass(this->vulkanSharedContext(), renderPassDesc,
                                                       compatibleOnly);
    if (!renderPass) {
        return nullptr;
    }

    renderPass->setKey(renderPassKey);
    fResourceCache->insertResource(renderPass.get());

    return renderPass;
}

VkPipelineCache VulkanResourceProvider::pipelineCache() {
    if (fPipelineCache == VK_NULL_HANDLE) {
        VkPipelineCacheCreateInfo createInfo;
        memset(&createInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;
        VkResult result;
        VULKAN_CALL_RESULT(this->vulkanSharedContext()->interface(),
                           result,
                           CreatePipelineCache(this->vulkanSharedContext()->device(),
                                               &createInfo,
                                               nullptr,
                                               &fPipelineCache));
        if (VK_SUCCESS != result) {
            fPipelineCache = VK_NULL_HANDLE;
        }
    }
    return fPipelineCache;
}

sk_sp<VulkanFramebuffer> VulkanResourceProvider::createFramebuffer(
        const VulkanSharedContext* context,
        const skia_private::TArray<VkImageView>& attachmentViews,
        const VulkanRenderPass& renderPass,
        const int width,
        const int height) {
    // TODO: Consider caching these in the future. If we pursue that, it may make more sense to
    // use a compatible renderpass rather than a full one to make each frame buffer more versatile.
    VkFramebufferCreateInfo framebufferInfo;
    memset(&framebufferInfo, 0, sizeof(VkFramebufferCreateInfo));
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = renderPass.renderPass();
    framebufferInfo.attachmentCount = attachmentViews.size();
    framebufferInfo.pAttachments = attachmentViews.begin();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    return VulkanFramebuffer::Make(context, framebufferInfo);
}

void VulkanResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kVulkan);

    skgpu::VulkanMemory::FreeImageMemory(
            this->vulkanSharedContext()->memoryAllocator(), *(texture.getMemoryAlloc()));

    VULKAN_CALL(this->vulkanSharedContext()->interface(),
                DestroyImage(this->vulkanSharedContext()->device(), texture.getVkImage(),
                             /*VkAllocationCallbacks=*/nullptr));
}

sk_sp<VulkanSamplerYcbcrConversion>
        VulkanResourceProvider::findOrCreateCompatibleSamplerYcbcrConversion(
                const VulkanYcbcrConversionInfo& ycbcrInfo) const {
    if (!ycbcrInfo.isValid()) {
        return nullptr;
    }
    auto ycbcrConversionKey = VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
            this->vulkanSharedContext(), ycbcrInfo);

    if (Resource* resource = fResourceCache->findAndRefResource(ycbcrConversionKey,
                                                                skgpu::Budgeted::kNo)) {
        return sk_sp<VulkanSamplerYcbcrConversion>(
                static_cast<VulkanSamplerYcbcrConversion*>(resource));
    }

    auto ycbcrConversion = VulkanSamplerYcbcrConversion::Make(this->vulkanSharedContext(),
                                                              ycbcrInfo);
    if (!ycbcrConversion) {
        return nullptr;
    }

    ycbcrConversion->setKey(ycbcrConversionKey);
    fResourceCache->insertResource(ycbcrConversion.get());

    return ycbcrConversion;
}

} // namespace skgpu::graphite
