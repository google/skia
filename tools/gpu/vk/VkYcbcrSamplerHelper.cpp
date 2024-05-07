/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

#ifdef SK_VULKAN

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanInterface.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#endif

int VkYcbcrSamplerHelper::GetExpectedY(int x, int y, int width, int height) {
    return 16 + (x + y) * 219 / (width + height - 2);
}

std::pair<int, int> VkYcbcrSamplerHelper::GetExpectedUV(int x, int y, int width, int height) {
    return { 16 + x * 224 / (width - 1), 16 + y * 224 / (height - 1) };
}

namespace {

void populate_ycbcr_image_info(VkImageCreateInfo* outImageInfo, uint32_t width, uint32_t height) {
    outImageInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    outImageInfo->pNext = nullptr;
    outImageInfo->flags = 0;
    outImageInfo->imageType = VK_IMAGE_TYPE_2D;
    outImageInfo->format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    outImageInfo->extent = VkExtent3D{width, height, 1};
    outImageInfo->mipLevels = 1;
    outImageInfo->arrayLayers = 1;
    outImageInfo->samples = VK_SAMPLE_COUNT_1_BIT;
    outImageInfo->tiling = VK_IMAGE_TILING_LINEAR;
    outImageInfo->usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    outImageInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    outImageInfo->queueFamilyIndexCount = 0;
    outImageInfo->pQueueFamilyIndices = nullptr;
    outImageInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

bool find_memory_type_index(const VkPhysicalDeviceMemoryProperties& phyDevMemProps,
                            const VkMemoryRequirements& memoryRequirements,
                            uint32_t* memoryTypeIndex) {
    for (uint32_t i = 0; i < phyDevMemProps.memoryTypeCount; ++i) {
        if (memoryRequirements.memoryTypeBits & (1 << i)) {
            // Map host-visible memory.
            if (phyDevMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                *memoryTypeIndex = i;
                return true;
            }
        }
    }
    return false;
}

}

#ifdef SK_GRAPHITE
// TODO(b/339211930): When graphite and ganesh can share a macro for certain Vulkan driver calls,
// much more code can be shared between this method and createGrBackendTexture.
bool VkYcbcrSamplerHelper::createBackendTexture(uint32_t width, uint32_t height) {
    // Create YCbCr image.
    VkImageCreateInfo vkImageInfo;
    populate_ycbcr_image_info(&vkImageInfo, width, height);
    SkASSERT(fImage == VK_NULL_HANDLE);

    VkResult result;
    VULKAN_CALL_RESULT(fSharedCtxt, result, CreateImage(fSharedCtxt->device(),
                                                        &vkImageInfo,
                                                        /*pAllocator=*/nullptr,
                                                        &fImage));
    if (result != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements requirements;
    VULKAN_CALL(fSharedCtxt->interface(), GetImageMemoryRequirements(fSharedCtxt->device(),
                                                                     fImage,
                                                                     &requirements));
    uint32_t memoryTypeIndex = 0;
    const VkPhysicalDeviceMemoryProperties& phyDevMemProps =
            fSharedCtxt->vulkanCaps().physicalDeviceMemoryProperties2().memoryProperties;
    if (!find_memory_type_index(phyDevMemProps, requirements, &memoryTypeIndex)) {
        return false;
    }

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    SkASSERT(fImageMemory == VK_NULL_HANDLE);
    VULKAN_CALL_RESULT(fSharedCtxt, result, AllocateMemory(fSharedCtxt->device(),
                                                           &allocInfo,
                                                           nullptr,
                                                           &fImageMemory));
    if (result != VK_SUCCESS) {
        return false;
    }

    void* mappedBuffer;
    VULKAN_CALL_RESULT(fSharedCtxt, result, MapMemory(fSharedCtxt->device(),
                                                      fImageMemory,
                                                      /*offset=*/0u,
                                                      requirements.size,
                                                      /*flags=*/0u,
                                                      &mappedBuffer));
    if (result != VK_SUCCESS) {
        return false;
    }

    // Write Y channel.
    VkImageSubresource subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout yLayout;
    VULKAN_CALL(fSharedCtxt->interface(),
                GetImageSubresourceLayout(fSharedCtxt->device(), fImage, &subresource, &yLayout));
    uint8_t* bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + yLayout.offset;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            bufferData[y * yLayout.rowPitch + x] = GetExpectedY(x, y, width, height);
        }
    }

    // Write UV channels.
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    VkSubresourceLayout uvLayout;
    VULKAN_CALL(fSharedCtxt->interface(), GetImageSubresourceLayout(fSharedCtxt->device(),
                                                                    fImage,
                                                                    &subresource,
                                                                    &uvLayout));
    bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + uvLayout.offset;
    for (size_t y = 0; y < height / 2; ++y) {
        for (size_t x = 0; x < width / 2; ++x) {
            auto [u, v] = GetExpectedUV(2*x, 2*y, width, height);
            bufferData[y * uvLayout.rowPitch + x * 2] = u;
            bufferData[y * uvLayout.rowPitch + x * 2 + 1] = v;
        }
    }

    VkMappedMemoryRange flushRange;
    flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flushRange.pNext = nullptr;
    flushRange.memory = fImageMemory;
    flushRange.offset = 0;
    flushRange.size = VK_WHOLE_SIZE;
    VULKAN_CALL_RESULT(fSharedCtxt, result,  FlushMappedMemoryRanges(fSharedCtxt->device(),
                                                                     /*memoryRangeCount=*/1,
                                                                     &flushRange));
    if (result != VK_SUCCESS) {
        return false;
    }
    VULKAN_CALL(fSharedCtxt->interface(), UnmapMemory(fSharedCtxt->device(), fImageMemory));

    // Bind image memory.
    VULKAN_CALL_RESULT(fSharedCtxt, result, BindImageMemory(fSharedCtxt->device(),
                                                            fImage,
                                                            fImageMemory,
                                                            /*memoryOffset=*/0u));
    if (result != VK_SUCCESS) {
        return false;
    }

    // Wrap the image into SkImage.
    VkFormatProperties formatProperties;
    SkASSERT(fPhysDev != VK_NULL_HANDLE);
    VULKAN_CALL(fSharedCtxt->interface(),
                GetPhysicalDeviceFormatProperties(fPhysDev,
                                                  VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                  &formatProperties));
    SkDEBUGCODE(auto linFlags = formatProperties.linearTilingFeatures;)
    SkASSERT((linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT));

    skgpu::VulkanYcbcrConversionInfo ycbcrInfo = {vkImageInfo.format,
                                                  /*externalFormat=*/0,
                                                  VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                                  VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                                  VK_CHROMA_LOCATION_COSITED_EVEN,
                                                  VK_CHROMA_LOCATION_COSITED_EVEN,
                                                  VK_FILTER_LINEAR,
                                                  false,
                                                  formatProperties.linearTilingFeatures};
    skgpu::VulkanAlloc alloc;
    alloc.fMemory = fImageMemory;
    alloc.fOffset = 0;
    alloc.fSize = requirements.size;

    skgpu::graphite::VulkanTextureInfo imageInfo = {
            static_cast<uint32_t>(vkImageInfo.samples),
            skgpu::Mipmapped::kNo,
            VK_IMAGE_CREATE_PROTECTED_BIT,
            vkImageInfo.format,
            vkImageInfo.tiling,
            vkImageInfo.usage,
            vkImageInfo.sharingMode,
            VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT,
            ycbcrInfo};

    fTexture = skgpu::graphite::BackendTexture{{(int32_t)width, (int32_t)height},
                                               imageInfo,
                                               VK_IMAGE_LAYOUT_UNDEFINED,
                                               /*queueFamilyIndex=*/0,
                                               fImage,
                                               alloc};
    return true;
}
#endif // SK_GRAPHITE

bool VkYcbcrSamplerHelper::createGrBackendTexture(uint32_t width, uint32_t height) {
    GrVkGpu* vkGpu = this->vkGpu();
    VkResult result;

    // Create YCbCr image.
    VkImageCreateInfo vkImageInfo;
    populate_ycbcr_image_info(&vkImageInfo, width, height);
    SkASSERT(fImage == VK_NULL_HANDLE);

    GR_VK_CALL_RESULT(vkGpu, result, CreateImage(vkGpu->device(), &vkImageInfo, nullptr, &fImage));
    if (result != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements requirements;
    GR_VK_CALL(vkGpu->vkInterface(), GetImageMemoryRequirements(vkGpu->device(),
                                                                fImage,
                                                                &requirements));

    uint32_t memoryTypeIndex = 0;
    VkPhysicalDeviceMemoryProperties phyDevMemProps;
    GR_VK_CALL(vkGpu->vkInterface(), GetPhysicalDeviceMemoryProperties(vkGpu->physicalDevice(),
                                                                       &phyDevMemProps));
    if (!find_memory_type_index(phyDevMemProps, requirements, &memoryTypeIndex)) {
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    SkASSERT(fImageMemory == VK_NULL_HANDLE);
    GR_VK_CALL_RESULT(vkGpu, result, AllocateMemory(vkGpu->device(), &allocInfo,
                                                    nullptr, &fImageMemory));
    if (result != VK_SUCCESS) {
        return false;
    }

    void* mappedBuffer;
    GR_VK_CALL_RESULT(vkGpu, result, MapMemory(vkGpu->device(), fImageMemory, 0u,
                                               requirements.size, 0u, &mappedBuffer));
    if (result != VK_SUCCESS) {
        return false;
    }

    // Write Y channel.
    VkImageSubresource subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout yLayout;
    GR_VK_CALL(vkGpu->vkInterface(), GetImageSubresourceLayout(vkGpu->device(), fImage,
                                                               &subresource, &yLayout));
    uint8_t* bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + yLayout.offset;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            bufferData[y * yLayout.rowPitch + x] = GetExpectedY(x, y, width, height);
        }
    }

    // Write UV channels.
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    VkSubresourceLayout uvLayout;
    GR_VK_CALL(vkGpu->vkInterface(), GetImageSubresourceLayout(vkGpu->device(), fImage,
                                                               &subresource, &uvLayout));
    bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + uvLayout.offset;
    for (size_t y = 0; y < height / 2; ++y) {
        for (size_t x = 0; x < width / 2; ++x) {
            auto [u, v] = GetExpectedUV(2*x, 2*y, width, height);
            bufferData[y * uvLayout.rowPitch + x * 2] = u;
            bufferData[y * uvLayout.rowPitch + x * 2 + 1] = v;
        }
    }

    VkMappedMemoryRange flushRange;
    flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flushRange.pNext = nullptr;
    flushRange.memory = fImageMemory;
    flushRange.offset = 0;
    flushRange.size = VK_WHOLE_SIZE;
    GR_VK_CALL_RESULT(vkGpu, result, FlushMappedMemoryRanges(vkGpu->device(), 1, &flushRange));
    if (result != VK_SUCCESS) {
        return false;
    }
    GR_VK_CALL(vkGpu->vkInterface(), UnmapMemory(vkGpu->device(), fImageMemory));

    // Bind image memory.
    GR_VK_CALL_RESULT(vkGpu, result, BindImageMemory(vkGpu->device(), fImage, fImageMemory, 0u));
    if (result != VK_SUCCESS) {
        return false;
    }

    // Wrap the image into SkImage.
    VkFormatProperties formatProperties;
    GR_VK_CALL(vkGpu->vkInterface(),
               GetPhysicalDeviceFormatProperties(vkGpu->physicalDevice(),
                                                 VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                 &formatProperties));
    SkDEBUGCODE(auto linFlags = formatProperties.linearTilingFeatures;)
    SkASSERT((linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) &&
             (linFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT));

    GrVkYcbcrConversionInfo ycbcrInfo = {vkImageInfo.format,
                                         /*externalFormat=*/0,
                                         VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                         VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                         VK_CHROMA_LOCATION_COSITED_EVEN,
                                         VK_CHROMA_LOCATION_COSITED_EVEN,
                                         VK_FILTER_LINEAR,
                                         false,
                                         formatProperties.linearTilingFeatures,
                                         /*fComponents=*/{}};
    skgpu::VulkanAlloc alloc;
    alloc.fMemory = fImageMemory;
    alloc.fOffset = 0;
    alloc.fSize = requirements.size;

    GrVkImageInfo imageInfo = {fImage,
                               alloc,
                               VK_IMAGE_TILING_LINEAR,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               vkImageInfo.format,
                               vkImageInfo.usage,
                               1 /* sample count */,
                               1 /* levelCount */,
                               VK_QUEUE_FAMILY_IGNORED,
                               GrProtected::kNo,
                               ycbcrInfo};

    fGrTexture = GrBackendTextures::MakeVk(width, height, imageInfo);
    return true;
}

GrVkGpu* VkYcbcrSamplerHelper::vkGpu() {
    return (GrVkGpu*) fDContext->priv().getGpu();
}

VkYcbcrSamplerHelper::VkYcbcrSamplerHelper(GrDirectContext* dContext) : fDContext(dContext) {
    SkASSERT_RELEASE(dContext->backend() == GrBackendApi::kVulkan);
}

VkYcbcrSamplerHelper::~VkYcbcrSamplerHelper() {
#ifdef SK_GRAPHITE
    if (fSharedCtxt) {
        if (fImage != VK_NULL_HANDLE) {
            VULKAN_CALL(fSharedCtxt->interface(),
                        DestroyImage(fSharedCtxt->device(), fImage, nullptr));
            fImage = VK_NULL_HANDLE;
        }
        if (fImageMemory != VK_NULL_HANDLE) {
            VULKAN_CALL(fSharedCtxt->interface(),
                        FreeMemory(fSharedCtxt->device(), fImageMemory, nullptr));
            fImageMemory = VK_NULL_HANDLE;
        }
    } else
#endif // SK_GRAPHITE
    {
        GrVkGpu* vkGpu = this->vkGpu();

        if (fImage != VK_NULL_HANDLE) {
            GR_VK_CALL(vkGpu->vkInterface(), DestroyImage(vkGpu->device(), fImage, nullptr));
            fImage = VK_NULL_HANDLE;
        }
        if (fImageMemory != VK_NULL_HANDLE) {
            GR_VK_CALL(vkGpu->vkInterface(), FreeMemory(vkGpu->device(), fImageMemory, nullptr));
            fImageMemory = VK_NULL_HANDLE;
        }
    }
}

bool VkYcbcrSamplerHelper::isYCbCrSupported() {
    VkFormatProperties formatProperties;
#ifdef SK_GRAPHITE
    if (fSharedCtxt) {
        if (!fSharedCtxt->vulkanCaps().supportsYcbcrConversion()) {
            return false;
        }

        SkASSERT(fPhysDev != VK_NULL_HANDLE);
        VULKAN_CALL(fSharedCtxt->interface(),
                    GetPhysicalDeviceFormatProperties(fPhysDev,
                                                    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                    &formatProperties));
    } else
#endif
    {
        GrVkGpu* vkGpu = this->vkGpu();
        if (!vkGpu->vkCaps().supportsYcbcrConversion()) {
            return false;
        }

        GR_VK_CALL(vkGpu->vkInterface(),
                GetPhysicalDeviceFormatProperties(vkGpu->physicalDevice(),
                                                    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                    &formatProperties));
    }

    // The createBackendTexture call (which is the point of this helper class) requires linear
    // support for VK_FORMAT_G8_B8R8_2PLANE_420_UNORM including sampling and cosited chroma.
    // Verify that the image format is supported.
    auto linFlags = formatProperties.linearTilingFeatures;
    if (!(linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ||
        !(linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ||
        !(linFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) ||
        !(linFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM is not supported
        return false;
    }
    return true;
}
#endif // SK_VULKAN
