/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

#ifdef SK_VULKAN

#include "include/gpu/GrContext.h"
#include "tools/gpu/vk/VkTestUtils.h"

int VkYcbcrSamplerHelper::GetExpectedY(int x, int y, int width, int height) {
    return 16 + (x + y) * 219 / (width + height - 2);
}

std::pair<int, int> VkYcbcrSamplerHelper::GetExpectedUV(int x, int y, int width, int height) {
    return { 16 + x * 224 / (width - 1), 16 + y * 224 / (height - 1) };
}

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        return false;                                                                        \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        return false;                                                                         \
    }

VkYcbcrSamplerHelper::VkYcbcrSamplerHelper() {}

VkYcbcrSamplerHelper::~VkYcbcrSamplerHelper() {
    fGrContext.reset();

    if (fImage != VK_NULL_HANDLE) {
        fVkDestroyImage(fDevice, fImage, nullptr);
        fImage = VK_NULL_HANDLE;
    }
    if (fImageMemory != VK_NULL_HANDLE) {
        fVkFreeMemory(fDevice, fImageMemory, nullptr);
        fImageMemory = VK_NULL_HANDLE;
    }

    fBackendContext.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext.fInstance, fDebugCallback, nullptr);
    }
    if (fBackendContext.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext.fInstance, nullptr);
        fBackendContext.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

bool VkYcbcrSamplerHelper::init() {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return false;
    }
    auto getProc = [&instProc, &devProc](const char* proc_name,
                                         VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(getProc, &fBackendContext, &fExtensions, &fFeatures,
                                             &fDebugCallback, nullptr, sk_gpu_test::CanPresentFn(),
                                             false)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT"));
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFormatProperties)
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties)

    ACQUIRE_DEVICE_VK_PROC(CreateImage)
    ACQUIRE_DEVICE_VK_PROC(DestroyImage)
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements)
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory)
    ACQUIRE_DEVICE_VK_PROC(FreeMemory)
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory)
    ACQUIRE_DEVICE_VK_PROC(MapMemory)
    ACQUIRE_DEVICE_VK_PROC(UnmapMemory)
    ACQUIRE_DEVICE_VK_PROC(FlushMappedMemoryRanges)
    ACQUIRE_DEVICE_VK_PROC(GetImageSubresourceLayout)

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    if (!fGrContext) {
        return false;
    }

    return true;
}

bool VkYcbcrSamplerHelper::isYCbCrSupported() const {
    bool ycbcrSupported = false;

    VkBaseOutStructure* feature = reinterpret_cast<VkBaseOutStructure*>(fFeatures.pNext);
    while (feature) {
        if (feature->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES) {
            VkPhysicalDeviceSamplerYcbcrConversionFeatures* ycbcrFeatures =
                    reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures*>(feature);
            ycbcrSupported = ycbcrFeatures->samplerYcbcrConversion;
            break;
        }
        feature = feature->pNext;
    }
    return ycbcrSupported;
}

sk_sp<SkImage> VkYcbcrSamplerHelper::createI420Image(uint32_t width, uint32_t height) {
    // Verify that the image format is supported.
    VkFormatProperties formatProperties;
    fVkGetPhysicalDeviceFormatProperties(fBackendContext.fPhysicalDevice,
                                         VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &formatProperties);
    if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM is not supported
        return nullptr;
    }

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
    vkImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SkASSERT(fImage == VK_NULL_HANDLE);
    if (fVkCreateImage(fDevice, &vkImageInfo, nullptr, &fImage) != VK_SUCCESS) {
        return nullptr;
    }

    VkMemoryRequirements requirements;
    fVkGetImageMemoryRequirements(fDevice, fImage, &requirements);

    uint32_t memoryTypeIndex = 0;
    bool foundHeap = false;
    VkPhysicalDeviceMemoryProperties phyDevMemProps;
    fVkGetPhysicalDeviceMemoryProperties(fBackendContext.fPhysicalDevice, &phyDevMemProps);
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
        return nullptr;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    SkASSERT(fImageMemory == VK_NULL_HANDLE);
    if (fVkAllocateMemory(fDevice, &allocInfo, nullptr, &fImageMemory) != VK_SUCCESS) {
        return nullptr;
    }

    void* mappedBuffer;
    if (fVkMapMemory(fDevice, fImageMemory, 0u, requirements.size, 0u, &mappedBuffer) !=
        VK_SUCCESS) {
        return nullptr;
    }

    // Write Y channel.
    VkImageSubresource subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout yLayout;
    fVkGetImageSubresourceLayout(fDevice, fImage, &subresource, &yLayout);
    uint8_t* bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + yLayout.offset;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            bufferData[y * yLayout.rowPitch + x] = GetExpectedY(x, y, width, height);
        }
    }

    // Write UV channels.
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    VkSubresourceLayout uvLayout;
    fVkGetImageSubresourceLayout(fDevice, fImage, &subresource, &uvLayout);
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
    if (fVkFlushMappedMemoryRanges(fDevice, 1, &flushRange) != VK_SUCCESS) {
        return nullptr;
    }
    fVkUnmapMemory(fDevice, fImageMemory);

    // Bind image memory.
    if (fVkBindImageMemory(fDevice, fImage, fImageMemory, 0u) != VK_SUCCESS) {
        return nullptr;
    }

    // Wrap the image into SkImage.
    GrVkYcbcrConversionInfo ycbcrInfo(vkImageInfo.format,
                                      /*externalFormat=*/0,
                                      VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                      VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                      VK_CHROMA_LOCATION_COSITED_EVEN,
                                      VK_CHROMA_LOCATION_COSITED_EVEN,
                                      VK_FILTER_LINEAR,
                                      false,
                                      formatProperties.linearTilingFeatures);
    GrVkAlloc alloc(fImageMemory, 0 /* offset */, requirements.size, 0 /* flags */);
    GrVkImageInfo imageInfo(fImage, alloc, VK_IMAGE_TILING_LINEAR, VK_IMAGE_LAYOUT_UNDEFINED,
                            vkImageInfo.format, 1 /* levelCount */, VK_QUEUE_FAMILY_IGNORED,
                            GrProtected::kNo, ycbcrInfo);

    fTexture = GrBackendTexture(width, height, imageInfo);
    sk_sp<SkImage> image = SkImage::MakeFromTexture(fGrContext.get(),
                                                    fTexture,
                                                    kTopLeft_GrSurfaceOrigin,
                                                    kRGB_888x_SkColorType,
                                                    kPremul_SkAlphaType,
                                                    nullptr);

    if (!image) {
        return nullptr;
    }

    return image;
}

#endif // SK_VULKAN
