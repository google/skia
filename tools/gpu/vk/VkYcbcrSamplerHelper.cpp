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

int VkYcbcrSamplerHelper::GetExpectedY(int x, int y, int width, int height) {
    return 16 + (x + y) * 219 / (width + height - 2);
}

std::pair<int, int> VkYcbcrSamplerHelper::GetExpectedUV(int x, int y, int width, int height) {
    return { 16 + x * 224 / (width - 1), 16 + y * 224 / (height - 1) };
}

GrVkGpu* VkYcbcrSamplerHelper::vkGpu() {
    return (GrVkGpu*) fDContext->priv().getGpu();
}

VkYcbcrSamplerHelper::VkYcbcrSamplerHelper(GrDirectContext* dContext) : fDContext(dContext) {
    SkASSERT_RELEASE(dContext->backend() == GrBackendApi::kVulkan);
}

VkYcbcrSamplerHelper::~VkYcbcrSamplerHelper() {
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

bool VkYcbcrSamplerHelper::isYCbCrSupported() {
    GrVkGpu* vkGpu = this->vkGpu();

    if (!vkGpu->vkCaps().supportsYcbcrConversion()) {
        return false;
    }

    // The createBackendTexture call (which is the point of this helper class) requires linear
    // support for VK_FORMAT_G8_B8R8_2PLANE_420_UNORM including sampling and cosited chroma.
    // Verify that the image format is supported.
    VkFormatProperties formatProperties;
    GR_VK_CALL(vkGpu->vkInterface(),
               GetPhysicalDeviceFormatProperties(vkGpu->physicalDevice(),
                                                 VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                 &formatProperties));
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

bool VkYcbcrSamplerHelper::createBackendTexture(uint32_t width, uint32_t height) {
    GrVkGpu* vkGpu = this->vkGpu();
    VkResult result;

    // Create YCbCr image.
    VkImageCreateInfo vkImageInfo = {};
    vkImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageInfo.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    vkImageInfo.extent = VkExtent3D{width, height, 1};
    vkImageInfo.mipLevels = 1;
    vkImageInfo.arrayLayers = 1;
    vkImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    vkImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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
    bool foundHeap = false;
    VkPhysicalDeviceMemoryProperties phyDevMemProps;
    GR_VK_CALL(vkGpu->vkInterface(), GetPhysicalDeviceMemoryProperties(vkGpu->physicalDevice(),
                                                                       &phyDevMemProps));
    for (uint32_t i = 0; i < phyDevMemProps.memoryTypeCount && !foundHeap; ++i) {
        if (requirements.memoryTypeBits & (1 << i)) {
            // Map host-visible memory.
            if (phyDevMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                memoryTypeIndex = i;
                foundHeap = true;
            }
        }
    }
    if (!foundHeap) {
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

    fTexture = GrBackendTextures::MakeVk(width, height, imageInfo);
    return true;
}

#endif // SK_VULKAN
