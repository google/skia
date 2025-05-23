/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/core/SkSpan.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanFramebuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/sksl/SkSLCompiler.h"

#ifdef  SK_BUILD_FOR_ANDROID
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include <android/hardware_buffer.h>
#endif

namespace skgpu::graphite {

constexpr int kMaxNumberOfCachedBufferDescSets = 1024;

namespace {

// Create a mock pipeline layout that has a compatible input attachment descriptor set layout and
// push constant parameters with all other real pipeline layouts. This allows us to perform
// once-per-renderpass operations even before a real pipeline is bound by the command buffer.
VkPipelineLayout create_mock_layout(const VulkanSharedContext* sharedContext) {
    SkASSERT(sharedContext);
    VkPushConstantRange pushConstantRange;
    pushConstantRange.offset = 0;
    pushConstantRange.size = VulkanResourceProvider::kIntrinsicConstantSize;
    pushConstantRange.stageFlags = VulkanResourceProvider::kIntrinsicConstantStageFlags;

    skia_private::STArray<1, DescriptorData> inputDesc {
            VulkanGraphicsPipeline::kInputAttachmentDescriptor};
    VkDescriptorSetLayout setLayout;
    DescriptorDataToVkDescSetLayout(sharedContext, inputDesc, &setLayout);

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 1;
    layoutCreateInfo.pSetLayouts = &setLayout;
    layoutCreateInfo.pushConstantRangeCount = 1;
    layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    VkResult result;
    VkPipelineLayout pipelineLayout;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreatePipelineLayout(sharedContext->device(),
                                            &layoutCreateInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &pipelineLayout));

    // Once the pipeline layout is created, we can clean up the VkDescriptorSetLayout.
    VULKAN_CALL(sharedContext->interface(),
                DestroyDescriptorSetLayout(sharedContext->device(), setLayout, nullptr));

    return pipelineLayout;
}

} // anonymous namespace

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID,
                                               size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fMockPipelineLayout(
                create_mock_layout(static_cast<const VulkanSharedContext*>(sharedContext)))
        , fUniformBufferDescSetCache(kMaxNumberOfCachedBufferDescSets) {}

VulkanResourceProvider::~VulkanResourceProvider() {
    if (fPipelineCache != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineCache(this->vulkanSharedContext()->device(),
                                         fPipelineCache,
                                         nullptr));
    }
    if (fMockPipelineLayout) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineLayout(this->vulkanSharedContext()->device(),
                                          fMockPipelineLayout,
                                          nullptr));
    }
}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() const {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    sk_sp<VulkanYcbcrConversion> ycbcrConversion;
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(texture.info());
    if (vkInfo.fYcbcrConversionInfo.isValid()) {
        ycbcrConversion = this->findOrCreateCompatibleYcbcrConversion(vkInfo.fYcbcrConversionInfo);
        if (!ycbcrConversion) {
            return nullptr;
        }
    }

    return VulkanTexture::MakeWrapped(this->vulkanSharedContext(),
                                      texture.dimensions(),
                                      texture.info(),
                                      BackendTextures::GetMutableState(texture),
                                      BackendTextures::GetVkImage(texture),
                                      /*alloc=*/{} /*Skia does not own wrapped texture memory*/,
                                      std::move(ycbcrConversion));
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        this,
                                        runtimeDict,
                                        pipelineKey,
                                        pipelineDesc,
                                        renderPassDesc,
                                        pipelineCreationFlags,
                                        compilationID);
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size,
                                                     const TextureInfo& info) {
    sk_sp<VulkanYcbcrConversion> ycbcrConversion;
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);
    if (vkInfo.fYcbcrConversionInfo.isValid()) {
        ycbcrConversion = this->findOrCreateCompatibleYcbcrConversion(vkInfo.fYcbcrConversionInfo);
        if (!ycbcrConversion) {
            return nullptr;
        }
    }

    return VulkanTexture::Make(this->vulkanSharedContext(),
                               size,
                               info,
                               std::move(ycbcrConversion));
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    sk_sp<VulkanYcbcrConversion> ycbcrConversion = nullptr;

    // Non-zero conversion information means the sampler utilizes a ycbcr conversion.
    const bool usesYcbcrConversion = samplerDesc.isImmutable();
    if (usesYcbcrConversion) {
        VulkanYcbcrConversionInfo ycbcrInfo = VulkanYcbcrConversion::FromImmutableSamplerInfo(
                samplerDesc.immutableSamplerInfo());
        ycbcrConversion = this->findOrCreateCompatibleYcbcrConversion(ycbcrInfo);
        if (!ycbcrConversion) {
            return nullptr;
        }
    }

    return VulkanSampler::Make(this->vulkanSharedContext(),
                               samplerDesc,
                               std::move(ycbcrConversion));
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo& info) {
    const auto& vkTexInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);
    VulkanTexture::CreatedImageInfo createdTextureInfo;
    if (!VulkanTexture::MakeVkImage(this->vulkanSharedContext(), dimensions, info,
                                    &createdTextureInfo)) {
        return {};
    }
    return BackendTextures::MakeVulkan(
            dimensions,
            vkTexInfo,
            skgpu::MutableTextureStates::GetVkImageLayout(createdTextureInfo.fMutableState.get()),
            skgpu::MutableTextureStates::GetVkQueueFamilyIndex(
                    createdTextureInfo.fMutableState.get()),
            createdTextureInfo.fImage,
            createdTextureInfo.fMemoryAlloc);
}

namespace {
GraphiteResourceKey build_desc_set_key(const SkSpan<DescriptorData>& requestedDescriptors) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    // The number of int32s needed for a key can depend on whether we use immutable samplers or not.
    // So, accumulte key data while passing through to check for that quantity and simply copy
    // into builder afterwards.
    skia_private::TArray<uint32_t> keyData (requestedDescriptors.size() + 1);

    keyData.push_back(requestedDescriptors.size());
    for (const DescriptorData& desc : requestedDescriptors) {
        keyData.push_back(static_cast<uint8_t>(desc.fType) << 24 |
                          desc.fBindingIndex << 16 |
                          static_cast<uint16_t>(desc.fCount));
        if (desc.fImmutableSampler) {
            const VulkanSampler* sampler =
                    static_cast<const VulkanSampler*>(desc.fImmutableSampler);
            SkASSERT(sampler);
            keyData.push_back_n(sampler->samplerDesc().asSpan().size(),
                                sampler->samplerDesc().asSpan().data());
        }
    }

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, keyData.size());

    for (int i = 0; i < keyData.size(); i++) {
        builder[i] = keyData[i];
    }

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
    resourceCache->insertResource(descSet.get(), descSetKey, Budgeted::kYes, Shareable::kNo);

    return descSet;
}
} // anonymous namespace

sk_sp<VulkanDescriptorSet> VulkanResourceProvider::findOrCreateDescriptorSet(
        SkSpan<DescriptorData> requestedDescriptors) {
    if (requestedDescriptors.empty()) {
        return nullptr;
    }

    // Search for available descriptor sets by assembling a key based upon the set's structure.
    GraphiteResourceKey key = build_desc_set_key(requestedDescriptors);
    if (auto descSet = fResourceCache->findAndRefResource(
                key, skgpu::Budgeted::kYes, Shareable::kNo)) {
        // A non-null resource pointer indicates we have found an available descriptor set.
        return sk_sp<VulkanDescriptorSet>(static_cast<VulkanDescriptorSet*>(descSet));
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
            add_new_desc_set_to_cache(context, pool, key, fResourceCache.get());
    if (!firstDescSet) {
        return nullptr;
    }

    // Continue to allocate & cache the maximum number of sets so they can be easily accessed as
    // they're needed.
    for (int i = 1; i < VulkanDescriptorPool::kMaxNumSets ; i++) {
        auto descSet =
                add_new_desc_set_to_cache(context, pool, key, fResourceCache.get());
        if (!descSet) {
            SKGPU_LOG_W("Descriptor set allocation %d of %d was unsuccessful; no more sets will be"
                        "allocated from this pool.", i, VulkanDescriptorPool::kMaxNumSets);
            break;
        }
    }

    return firstDescSet;
}

namespace {

VulkanResourceProvider::UniformBindGroupKey make_ubo_bind_group_key(
        SkSpan<DescriptorData> requestedDescriptors,
        SkSpan<BindBufferInfo> bindUniformBufferInfo) {
    VulkanResourceProvider::UniformBindGroupKey uniqueKey;
    {
        // Each entry in the bind group needs 2 uint32_t in the key:
        //  - buffer's unique ID: 32 bits.
        //  - buffer's binding size: 32 bits.
        // We need total of 4 entries in the uniform buffer bind group.
        // Unused entries will be assigned zero values.
        VulkanResourceProvider::UniformBindGroupKey::Builder builder(&uniqueKey);

        for (uint32_t i = 0; i < VulkanGraphicsPipeline::kNumUniformBuffers; ++i) {
            builder[2 * i] = 0;
            builder[2 * i + 1] = 0;
        }

        for (uint32_t i = 0; i < requestedDescriptors.size(); ++i) {
            int descriptorBindingIndex = requestedDescriptors[i].fBindingIndex;
            SkASSERT(SkTo<unsigned long>(descriptorBindingIndex) < bindUniformBufferInfo.size());
            SkASSERT(SkTo<unsigned long>(descriptorBindingIndex) <
                     VulkanGraphicsPipeline::kNumUniformBuffers);
            const auto& bindInfo = bindUniformBufferInfo[descriptorBindingIndex];
            const VulkanBuffer* boundBuffer = static_cast<const VulkanBuffer*>(bindInfo.fBuffer);
            SkASSERT(boundBuffer);
            builder[2 * descriptorBindingIndex] = boundBuffer->uniqueID().asUInt();
            builder[2 * descriptorBindingIndex + 1] = bindInfo.fSize;
        }

        builder.finish();
    }

    return uniqueKey;
}

void update_uniform_descriptor_set(SkSpan<DescriptorData> requestedDescriptors,
                                   SkSpan<BindBufferInfo> bindUniformBufferInfo,
                                   VkDescriptorSet descSet,
                                   const VulkanSharedContext* sharedContext) {
    for (size_t i = 0; i < requestedDescriptors.size(); i++) {
        int descriptorBindingIndex = requestedDescriptors[i].fBindingIndex;
        SkASSERT(SkTo<unsigned long>(descriptorBindingIndex) < bindUniformBufferInfo.size());
        const auto& bindInfo = bindUniformBufferInfo[descriptorBindingIndex];
        if (bindInfo.fBuffer) {
#if defined(SK_DEBUG)
            static uint64_t maxBufferRange =
                sharedContext->caps()->storageBufferSupport()
                    ? sharedContext->vulkanCaps().maxStorageBufferRange()
                    : sharedContext->vulkanCaps().maxUniformBufferRange();
            SkASSERT(bindInfo.fSize <= maxBufferRange);
#endif
            VkDescriptorBufferInfo bufferInfo = {};
            auto vulkanBuffer = static_cast<const VulkanBuffer*>(bindInfo.fBuffer);
            bufferInfo.buffer = vulkanBuffer->vkBuffer();
            bufferInfo.offset = 0; // We always use dynamic ubos so we set the base offset to 0
            bufferInfo.range = bindInfo.fSize;

            VkWriteDescriptorSet writeInfo = {};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.dstSet = descSet;
            writeInfo.dstBinding = descriptorBindingIndex;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = requestedDescriptors[i].fCount;
            writeInfo.descriptorType = DsTypeEnumToVkDs(requestedDescriptors[i].fType);
            writeInfo.pBufferInfo = &bufferInfo;

            // TODO(b/293925059): Migrate to updating all the uniform descriptors with one driver
            // call. Calling UpdateDescriptorSets once to encapsulate updates to all uniform
            // descriptors would be ideal, but that led to issues with draws where all the UBOs
            // within that set would unexpectedly be assigned the same offset. Updating them one at
            // a time within this loop works in the meantime but is suboptimal.
            VULKAN_CALL(sharedContext->interface(),
                        UpdateDescriptorSets(sharedContext->device(),
                                             /*descriptorWriteCount=*/1,
                                             &writeInfo,
                                             /*descriptorCopyCount=*/0,
                                             /*pDescriptorCopies=*/nullptr));
        }
    }
}

} // anonymous namespace

sk_sp<VulkanDescriptorSet> VulkanResourceProvider::findOrCreateUniformBuffersDescriptorSet(
        SkSpan<DescriptorData> requestedDescriptors,
        SkSpan<BindBufferInfo> bindUniformBufferInfo) {
    SkASSERT(requestedDescriptors.size() <= VulkanGraphicsPipeline::kNumUniformBuffers);

    auto key = make_ubo_bind_group_key(requestedDescriptors, bindUniformBufferInfo);
    auto* existingDescSet = fUniformBufferDescSetCache.find(key);
    if (existingDescSet) {
        return *existingDescSet;
    }
    sk_sp<VulkanDescriptorSet> newDS = this->findOrCreateDescriptorSet(requestedDescriptors);
    if (!newDS) {
        return nullptr;
    }

    update_uniform_descriptor_set(requestedDescriptors,
                                  bindUniformBufferInfo,
                                  *newDS->descriptorSet(),
                                  this->vulkanSharedContext());
    return *fUniformBufferDescSetCache.insert(key, newDS);
}

sk_sp<VulkanRenderPass> VulkanResourceProvider::findOrCreateRenderPass(
            const RenderPassDesc& renderPassDesc,
            bool compatibleOnly) {
    static constexpr Budgeted kBudgeted = Budgeted::kYes;
    static constexpr Shareable kShareable = Shareable::kYes;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    VulkanRenderPass::Metadata rpMetadata{renderPassDesc, compatibleOnly};
    GraphiteResourceKey key;
    {
        GraphiteResourceKey::Builder builder(&key, kType, rpMetadata.keySize());

        int startingIdx = 0;
        rpMetadata.addToKey(builder, startingIdx);
    }
    if (Resource* resource = fResourceCache->findAndRefResource(key, kBudgeted, kShareable)) {
        return sk_sp<VulkanRenderPass>(static_cast<VulkanRenderPass*>(resource));
    }

    sk_sp<VulkanRenderPass> renderPass =
            VulkanRenderPass::Make(this->vulkanSharedContext(), rpMetadata);
    if (!renderPass) {
        return nullptr;
    }

    fResourceCache->insertResource(renderPass.get(), key, kBudgeted, kShareable);

    return renderPass;
}

VkPipelineCache VulkanResourceProvider::pipelineCache() {
    if (fPipelineCache == VK_NULL_HANDLE) {
        VkPipelineCacheCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;
        VkResult result;
        VULKAN_CALL_RESULT(this->vulkanSharedContext(),
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

namespace {

void gather_attachment_views(skia_private::TArray<VkImageView>& attachmentViews,
                             VulkanTexture* colorTexture,
                             VulkanTexture* resolveTexture,
                             VulkanTexture* depthStencilTexture) {
    if (colorTexture) {
        VkImageView& colorAttachmentView = attachmentViews.push_back();
        colorAttachmentView =
                colorTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();

        if (resolveTexture) {
            VkImageView& resolveView = attachmentViews.push_back();
            resolveView =
                    resolveTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
        }
    }

    if (depthStencilTexture) {
        VkImageView& stencilView = attachmentViews.push_back();
        stencilView =
                depthStencilTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
    }
}

} // anonymous namespace

sk_sp<VulkanFramebuffer> VulkanResourceProvider::findOrCreateFramebuffer(
        const VulkanSharedContext* context,
        VulkanTexture* colorTexture,
        VulkanTexture* resolveTexture,
        VulkanTexture* depthStencilTexture,
        const RenderPassDesc& renderPassDesc,
        const VulkanRenderPass& renderPass,
        const int width,
        const int height) {

    VulkanTexture* mainTexture = nullptr;
    if (colorTexture) {
        mainTexture = resolveTexture ? resolveTexture : colorTexture;
    } else {
        SkASSERT(depthStencilTexture);
        mainTexture = depthStencilTexture;
    }
    SkASSERT(mainTexture);
    VulkanTexture* msaaTexture = resolveTexture ? colorTexture : nullptr;

    // First check for a cached frame buffer.
    sk_sp<VulkanFramebuffer> fb = mainTexture->getCachedFramebuffer(renderPassDesc,
                                                                    msaaTexture,
                                                                    depthStencilTexture);
    if (fb) {
        return fb;
    }

    // Gather attachment views neeeded for frame buffer creation.
    skia_private::TArray<VkImageView> attachmentViews;
    gather_attachment_views(attachmentViews, colorTexture, resolveTexture, depthStencilTexture);

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.renderPass();
    framebufferInfo.attachmentCount = attachmentViews.size();
    framebufferInfo.pAttachments = attachmentViews.begin();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    fb = VulkanFramebuffer::Make(context,
                                 framebufferInfo,
                                 renderPassDesc,
                                 sk_ref_sp(msaaTexture),
                                 sk_ref_sp(depthStencilTexture));
    mainTexture->addCachedFramebuffer(fb);
    return fb;
}

void VulkanResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kVulkan);

    VULKAN_CALL(this->vulkanSharedContext()->interface(),
                DestroyImage(this->vulkanSharedContext()->device(),
                             BackendTextures::GetVkImage(texture),
                             /*VkAllocationCallbacks=*/nullptr));

    VulkanAlloc alloc = BackendTextures::GetMemoryAlloc(texture);
    // Free the image memory used for the BackendTexture's VkImage.
    //
    // How we do this is dependent upon on how the image was allocated (via the memory allocator or
    // with a direct call to the Vulkan driver) . If the VulkanAlloc's fBackendMemory is != 0, then
    // that means the allocator was used. Otherwise, a direct driver call was used and we should
    // free the VkDeviceMemory (fMemory).
    if (alloc.fBackendMemory) {
        skgpu::VulkanMemory::FreeImageMemory(this->vulkanSharedContext()->memoryAllocator(), alloc);
    } else {
        SkASSERT(alloc.fMemory != VK_NULL_HANDLE);
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    FreeMemory(this->vulkanSharedContext()->device(), alloc.fMemory, nullptr));
    }
}

sk_sp<VulkanYcbcrConversion> VulkanResourceProvider::findOrCreateCompatibleYcbcrConversion(
        const VulkanYcbcrConversionInfo& ycbcrInfo) const {
    static constexpr Budgeted kBudgeted = Budgeted::kYes;
    static constexpr Shareable kShareable = Shareable::kYes;
    if (!ycbcrInfo.isValid()) {
        return nullptr;
    }

    GraphiteResourceKey key;
    {
        static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
        static constexpr int kKeySize = 3;

        GraphiteResourceKey::Builder builder(&key, kType, kKeySize);
        ImmutableSamplerInfo packedInfo = VulkanYcbcrConversion::ToImmutableSamplerInfo(ycbcrInfo);
        builder[0] = packedInfo.fNonFormatYcbcrConversionInfo;
        builder[1] = (uint32_t) packedInfo.fFormat;
        builder[2] = (uint32_t) (packedInfo.fFormat >> 32);
    }

    if (Resource* resource = fResourceCache->findAndRefResource(key, kBudgeted, kShareable)) {
        return sk_sp(static_cast<VulkanYcbcrConversion*>(resource));
    }

    auto ycbcrConversion = VulkanYcbcrConversion::Make(this->vulkanSharedContext(), ycbcrInfo);
    if (!ycbcrConversion) {
        return nullptr;
    }

    fResourceCache->insertResource(ycbcrConversion.get(), key, kBudgeted, kShareable);
    return ycbcrConversion;
}

sk_sp<VulkanGraphicsPipeline> VulkanResourceProvider::findOrCreateLoadMSAAPipeline(
        const RenderPassDesc& renderPassDesc) {
    if (renderPassDesc.fColorResolveAttachment.fFormat == TextureFormat::kUnsupported ||
        renderPassDesc.fColorAttachment.fFormat == TextureFormat::kUnsupported) {
        SKGPU_LOG_E("Loading MSAA from resolve texture requires valid color & resolve attachment");
        return nullptr;
    }

    // Check to see if we already have a suitable pipeline that we can use.
    VulkanRenderPass::Metadata rpMetadata{renderPassDesc, /*compatibleOnly=*/true};
    for (int i = 0; i < fLoadMSAAPipelines.size(); i++) {
        if (rpMetadata == fLoadMSAAPipelines.at(i).first) {
            return fLoadMSAAPipelines.at(i).second;
        }
    }

    if (!fLoadMSAAProgram) {
        // Lazily create the modules and pipeline layout the first time we need to load MSAA
        fLoadMSAAProgram =
                VulkanGraphicsPipeline::CreateLoadMSAAProgram(this->vulkanSharedContext());
        if (!fLoadMSAAProgram) {
            SKGPU_LOG_E("Failed to initialize MSAA load pipeline creation structure(s)");
            return nullptr;
        }
    }

    sk_sp<VulkanGraphicsPipeline> pipeline = VulkanGraphicsPipeline::MakeLoadMSAAPipeline(
            this->vulkanSharedContext(), this, *fLoadMSAAProgram, renderPassDesc);
    if (!pipeline) {
        SKGPU_LOG_E("Failed to create MSAA load pipeline");
        return nullptr;
    }

    fLoadMSAAPipelines.push_back(std::make_pair(rpMetadata, pipeline));
    return pipeline;
}

#ifdef SK_BUILD_FOR_ANDROID

BackendTexture VulkanResourceProvider::onCreateBackendTexture(AHardwareBuffer* hardwareBuffer,
                                                              bool isRenderable,
                                                              bool isProtectedContent,
                                                              SkISize dimensions,
                                                              bool fromAndroidWindow) const {

    const VulkanSharedContext* vkContext = this->vulkanSharedContext();
    VkDevice device = vkContext->device();
    const VulkanCaps& vkCaps = vkContext->vulkanCaps();

    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    if (!skgpu::GetAHardwareBufferProperties(
                &hwbFormatProps, &hwbProps, vkContext->interface(), hardwareBuffer, device)) {
        return {};
    }

    bool importAsExternalFormat = hwbFormatProps.format == VK_FORMAT_UNDEFINED;

    // Start to assemble VulkanTextureInfo which is needed later on to create the VkImage but can
    // sooner help us query VulkanCaps for certain format feature support.
    // TODO: Allow client to pass in tiling mode. For external formats, this is required to be
    // optimal. For AHB that have a known Vulkan format, we can query VulkanCaps to determine if
    // optimal is a valid decision given the format features.
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageCreateFlags imgCreateflags = isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    // When importing as an external format the image usage can only be VK_IMAGE_USAGE_SAMPLED_BIT.
    if (!importAsExternalFormat) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (isRenderable) {
            // Renderable attachments can be used as input attachments if we are loading from MSAA.
            usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
    }
    VulkanTextureInfo vkTexInfo { VK_SAMPLE_COUNT_1_BIT,
                                  Mipmapped::kNo,
                                  imgCreateflags,
                                  hwbFormatProps.format,
                                  tiling,
                                  usageFlags,
                                  VK_SHARING_MODE_EXCLUSIVE,
                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                  VulkanYcbcrConversionInfo() };

    if (isRenderable && (importAsExternalFormat || !vkCaps.isRenderable(vkTexInfo))) {
        SKGPU_LOG_W("Renderable texture requested from an AHardwareBuffer which uses a VkFormat "
                    "that Skia cannot render to (VkFormat: %d).\n",  hwbFormatProps.format);
        return {};
    }

    if (!importAsExternalFormat && (!vkCaps.isTransferSrc(vkTexInfo) ||
                                    !vkCaps.isTransferDst(vkTexInfo) ||
                                    !vkCaps.isTexturable(vkTexInfo))) {
        if (isRenderable) {
            SKGPU_LOG_W("VkFormat %d is either unfamiliar to Skia or doesn't support the necessary"
                        " format features. Because a renerable texture was requested, we cannot "
                        "fall back to importing with an external format.\n", hwbFormatProps.format);
            return {};
        }
        // If the VkFormat does not support the features we need, then import as an external format.
        importAsExternalFormat = true;
        // If we use VkExternalFormatANDROID with an externalFormat != 0, then format must =
        // VK_FORMAT_UNDEFINED.
        vkTexInfo.fFormat = VK_FORMAT_UNDEFINED;
        vkTexInfo.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    VulkanYcbcrConversionInfo ycbcrInfo;
    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.
    if (importAsExternalFormat) {
        GetYcbcrConversionInfoFromFormatProps(&ycbcrInfo, hwbFormatProps);
        if (!ycbcrInfo.isValid()) {
            SKGPU_LOG_W("Failed to create valid YCbCr conversion information from hardware buffer"
                        "format properties.\n");
            return {};
        }
        vkTexInfo.fYcbcrConversionInfo = ycbcrInfo;
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }
    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };

    SkASSERT(!(vkTexInfo.fFlags & VK_IMAGE_CREATE_PROTECTED_BIT) ||
             fSharedContext->isProtected() == Protected::kYes);

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                                 // sType
        &externalMemoryImageInfo,                                            // pNext
        vkTexInfo.fFlags,                                                    // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                                                    // VkImageType
        vkTexInfo.fFormat,                                                   // VkFormat
        { (uint32_t)dimensions.fWidth, (uint32_t)dimensions.fHeight, 1 },    // VkExtent3D
        1,                                                                   // mipLevels
        1,                                                                   // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                                               // samples
        vkTexInfo.fImageTiling,                                              // VkImageTiling
        vkTexInfo.fImageUsageFlags,                                          // VkImageUsageFlags
        vkTexInfo.fSharingMode,                                              // VkSharingMode
        0,                                                                   // queueFamilyCount
        nullptr,                                                             // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                                           // initialLayout
    };

    VkResult result;
    VkImage image;
    VULKAN_CALL_RESULT(vkContext, result, CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return {};
    }

    const VkPhysicalDeviceMemoryProperties2& phyDevMemProps =
            vkContext->vulkanCaps().physicalDeviceMemoryProperties2();
    VulkanAlloc alloc;
    if (!AllocateAndBindImageMemory(&alloc, image, phyDevMemProps, hwbProps, hardwareBuffer,
                                    vkContext->interface(), device)) {
        VULKAN_CALL(vkContext->interface(), DestroyImage(device, image, nullptr));
        return {};
    }

    return BackendTextures::MakeVulkan(dimensions,
                                       vkTexInfo,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_QUEUE_FAMILY_FOREIGN_EXT,
                                       image,
                                       alloc);
}

#endif // SK_BUILD_FOR_ANDROID

} // namespace skgpu::graphite
