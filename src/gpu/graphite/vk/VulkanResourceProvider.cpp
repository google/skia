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

#ifdef  SK_BUILD_FOR_ANDROID
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include <android/hardware_buffer.h>
#endif

namespace skgpu::graphite {

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID,
                                               size_t resourceBudget,
                                               sk_sp<Buffer> intrinsicConstantUniformBuffer,
                                               sk_sp<Buffer> loadMSAAVertexBuffer)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fIntrinsicUniformBuffer(std::move(intrinsicConstantUniformBuffer))
        , fLoadMSAAVertexBuffer(std::move(loadMSAAVertexBuffer)) {
}

VulkanResourceProvider::~VulkanResourceProvider() {
    if (fPipelineCache != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineCache(this->vulkanSharedContext()->device(),
                                         fPipelineCache,
                                         nullptr));
    }
    if (fMSAALoadVertShaderModule != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyShaderModule(this->vulkanSharedContext()->device(),
                                        fMSAALoadVertShaderModule,
                                        nullptr));
    }
    if (fMSAALoadFragShaderModule != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyShaderModule(this->vulkanSharedContext()->device(),
                                        fMSAALoadFragShaderModule,
                                        nullptr));
    }
    if (fMSAALoadPipelineLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineLayout(this->vulkanSharedContext()->device(),
                                          fMSAALoadPipelineLayout,
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
                                      /*alloc=*/{},  // Skia does not own wrapped texture memory
                                      "WrappedTexture");
}

sk_sp<Buffer> VulkanResourceProvider::refIntrinsicConstantBuffer() const {
    return fIntrinsicUniformBuffer;
}

const Buffer* VulkanResourceProvider::loadMSAAVertexBuffer() const {
    return fLoadMSAAVertexBuffer.get();
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    auto compatibleRenderPass =
            this->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/true);
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        runtimeDict,
                                        pipelineDesc,
                                        renderPassDesc,
                                        compatibleRenderPass,
                                        this->pipelineCache());
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size,
                                                     const TextureInfo& info,
                                                     std::string_view label,
                                                     skgpu::Budgeted budgeted) {
    return VulkanTexture::Make(this->vulkanSharedContext(),
                               this,
                               size,
                               info,
                               std::move(label),
                               budgeted);
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern,
                                                   std::string_view label) {
    return VulkanBuffer::Make(this->vulkanSharedContext(),
                              size,
                              type,
                              accessPattern,
                              std::move(label));
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return VulkanSampler::Make(this->vulkanSharedContext(),
                               samplerDesc.samplingOptions(),
                               samplerDesc.tileModeX(),
                               samplerDesc.tileModeY());
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
            skgpu::MutableTextureStates::GetVkImageLayout(createdTextureInfo.fMutableState.get()),
            skgpu::MutableTextureStates::GetVkQueueFamilyIndex(createdTextureInfo.fMutableState.get()),
            createdTextureInfo.fImage,
            createdTextureInfo.fMemoryAlloc};
}

namespace {
GraphiteResourceKey build_desc_set_key(const SkSpan<DescriptorData>& requestedDescriptors) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    const int num32DataCnt = requestedDescriptors.size() + 1;

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    builder[0] = requestedDescriptors.size();
    for (int i = 1; i < num32DataCnt; i++) {
        const auto& currDesc = requestedDescriptors[i - 1];
        // TODO: Consider making the DescriptorData struct itself just use uint16_t.
        uint16_t smallerCount = static_cast<uint16_t>(currDesc.count);
        builder[i] = static_cast<uint8_t>(currDesc.type) << 24 |
                     currDesc.bindingIndex << 16 |
                     smallerCount;
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
    // Search for available descriptor sets by assembling a key based upon the set's structure.
    GraphiteResourceKey key = build_desc_set_key(requestedDescriptors);
    if (auto descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kYes)) {
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

sk_sp<VulkanRenderPass> VulkanResourceProvider::findOrCreateRenderPassWithKnownKey(
            const RenderPassDesc& renderPassDesc,
            bool compatibleOnly,
            const GraphiteResourceKey& rpKey) {
    if (Resource* resource =
            fResourceCache->findAndRefResource(rpKey, skgpu::Budgeted::kYes)) {
        return sk_sp<VulkanRenderPass>(static_cast<VulkanRenderPass*>(resource));
    }

    sk_sp<VulkanRenderPass> renderPass =
                VulkanRenderPass::MakeRenderPass(this->vulkanSharedContext(),
                                                 renderPassDesc,
                                                 compatibleOnly);
    if (!renderPass) {
        return nullptr;
    }

    renderPass->setKey(rpKey);
    fResourceCache->insertResource(renderPass.get());

    return renderPass;
}

sk_sp<VulkanRenderPass> VulkanResourceProvider::findOrCreateRenderPass(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {
    GraphiteResourceKey rpKey = VulkanRenderPass::MakeRenderPassKey(renderPassDesc, compatibleOnly);

    return this->findOrCreateRenderPassWithKnownKey(renderPassDesc, compatibleOnly, rpKey);
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

    VULKAN_CALL(this->vulkanSharedContext()->interface(),
                DestroyImage(this->vulkanSharedContext()->device(), texture.getVkImage(),
                             /*VkAllocationCallbacks=*/nullptr));

    // Free the image memory used for the BackendTexture's VkImage.
    //
    // How we do this is dependent upon on how the image was allocated (via the memory allocator or
    // with a direct call to the Vulkan driver) . If the VulkanAlloc's fBackendMemory is != 0, then
    // that means the allocator was used. Otherwise, a direct driver call was used and we should
    // free the VkDeviceMemory (fMemory).
    if (texture.getMemoryAlloc()->fBackendMemory) {
        skgpu::VulkanMemory::FreeImageMemory(this->vulkanSharedContext()->memoryAllocator(),
                                             *(texture.getMemoryAlloc()));
    } else {
        SkASSERT(texture.getMemoryAlloc()->fMemory != VK_NULL_HANDLE);
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    FreeMemory(this->vulkanSharedContext()->device(),
                               texture.getMemoryAlloc()->fMemory,
                               nullptr));
    }
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

sk_sp<VulkanGraphicsPipeline> VulkanResourceProvider::findOrCreateLoadMSAAPipeline(
        const RenderPassDesc& renderPassDesc) {

    if (!renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() ||
        !renderPassDesc.fColorAttachment.fTextureInfo.isValid()) {
        SKGPU_LOG_E("Loading MSAA from resolve texture requires valid color & resolve attachment");
        return nullptr;
    }

    // Check to see if we already have a suitable pipeline that we can use.
    GraphiteResourceKey renderPassKey =
            VulkanRenderPass::MakeRenderPassKey(renderPassDesc, /*compatibleOnly=*/true);
    for (int i = 0; i < fLoadMSAAPipelines.size(); i++) {
        if (renderPassKey == fLoadMSAAPipelines.at(i).first) {
            return fLoadMSAAPipelines.at(i).second;
        }
    }

    // If any of the load MSAA pipeline creation structures are null then we need to initialize
    // those before proceeding. If the creation of one of them fails, all are assigned to null, so
    // we only need to check one of the structures.
    if (fMSAALoadVertShaderModule   == VK_NULL_HANDLE) {
        SkASSERT(fMSAALoadFragShaderModule  == VK_NULL_HANDLE &&
                 fMSAALoadPipelineLayout == VK_NULL_HANDLE);
        if (!VulkanGraphicsPipeline::InitializeMSAALoadPipelineStructs(
                    this->vulkanSharedContext(),
                    &fMSAALoadVertShaderModule,
                    &fMSAALoadFragShaderModule,
                    &fMSAALoadShaderStageInfo[0],
                    &fMSAALoadPipelineLayout)) {
            SKGPU_LOG_E("Failed to initialize MSAA load pipeline creation structure(s)");
            return nullptr;
        }
    }

    sk_sp<VulkanRenderPass> compatibleRenderPass =
            this->findOrCreateRenderPassWithKnownKey(renderPassDesc,
                                                     /*compatibleOnly=*/true,
                                                     renderPassKey);
    if (!compatibleRenderPass) {
        SKGPU_LOG_E("Failed to make compatible render pass for loading MSAA");
    }

    sk_sp<VulkanGraphicsPipeline> pipeline = VulkanGraphicsPipeline::MakeLoadMSAAPipeline(
            this->vulkanSharedContext(),
            fMSAALoadVertShaderModule,
            fMSAALoadFragShaderModule,
            &fMSAALoadShaderStageInfo[0],
            fMSAALoadPipelineLayout,
            compatibleRenderPass,
            this->pipelineCache(),
            renderPassDesc.fColorAttachment.fTextureInfo);

    if (!pipeline) {
        SKGPU_LOG_E("Failed to create MSAA load pipeline");
        return nullptr;
    }

    fLoadMSAAPipelines.push_back(std::make_pair(renderPassKey, pipeline));
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
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling mode.
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
    result = VULKAN_CALL(vkContext->interface(),
                         CreateImage(device, &imageCreateInfo, nullptr, &image));
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

    return { dimensions, vkTexInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_FOREIGN_EXT,
             image, alloc};
}

#endif // SK_BUILD_FOR_ANDROID

} // namespace skgpu::graphite
