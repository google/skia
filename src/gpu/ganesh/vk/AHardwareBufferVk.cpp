/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/GrAHardwareBufferUtils.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <android/hardware_buffer.h>

#define VK_CALL(X) gpu->vkInterface()->fFunctions.f##X

namespace GrAHardwareBufferUtils {

GrBackendFormat GetVulkanBackendFormat(GrDirectContext* dContext, AHardwareBuffer* hardwareBuffer,
                                       uint32_t bufferFormat, bool requireKnownFormat) {
    GrBackendApi backend = dContext->backend();
    if (backend != GrBackendApi::kVulkan) {
        return GrBackendFormat();
    }
    switch (bufferFormat) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return GrBackendFormats::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT);
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_R5G6B5_UNORM_PACK16);
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8_UNORM);
#if __ANDROID_API__ >= 33
        case AHARDWAREBUFFER_FORMAT_R8_UNORM:
            return GrBackendFormats::MakeVk(VK_FORMAT_R8_UNORM);
#endif
        default: {
            if (requireKnownFormat) {
                return GrBackendFormat();
            }
            GrVkGpu* gpu = static_cast<GrVkGpu*>(dContext->priv().getGpu());
            SkASSERT(gpu);
            VkDevice device = gpu->device();

            if (!gpu->vkCaps().supportsAndroidHWBExternalMemory()) {
                return GrBackendFormat();
            }

            VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
            VkAndroidHardwareBufferPropertiesANDROID hwbProps;
            if (!GetAHardwareBufferProperties(
                        &hwbFormatProps, &hwbProps, gpu->vkInterface(), hardwareBuffer, device) ||
                hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
                return GrBackendFormat();
            }

            GrVkYcbcrConversionInfo ycbcrConversion;
            GetYcbcrConversionInfoFromFormatProps(&ycbcrConversion, hwbFormatProps);

            return GrBackendFormats::MakeVk(ycbcrConversion);
        }
    }
}

class VulkanCleanupHelper {
public:
    VulkanCleanupHelper(GrVkGpu* gpu, VkImage image, VkDeviceMemory memory)
        : fDevice(gpu->device())
        , fImage(image)
        , fMemory(memory)
        , fDestroyImage(gpu->vkInterface()->fFunctions.fDestroyImage)
        , fFreeMemory(gpu->vkInterface()->fFunctions.fFreeMemory) {}
    ~VulkanCleanupHelper() {
        fDestroyImage(fDevice, fImage, nullptr);
        fFreeMemory(fDevice, fMemory, nullptr);
    }
private:
    VkDevice           fDevice;
    VkImage            fImage;
    VkDeviceMemory     fMemory;
    PFN_vkDestroyImage fDestroyImage;
    PFN_vkFreeMemory   fFreeMemory;
};

void delete_vk_image(void* context) {
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    delete cleanupHelper;
}

void update_vk_image(void* context, GrDirectContext* dContext) {
    // no op
}

static GrBackendTexture make_vk_backend_texture(
        GrDirectContext* dContext, AHardwareBuffer* hardwareBuffer,
        int width, int height,
        DeleteImageProc* deleteProc,
        UpdateImageProc* updateProc,
        TexImageCtx* imageCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat,
        bool isRenderable,
        bool fromAndroidWindow) {
    SkASSERT(dContext->backend() == GrBackendApi::kVulkan);
    GrVkGpu* gpu = static_cast<GrVkGpu*>(dContext->priv().getGpu());

    SkASSERT(!isProtectedContent || gpu->protectedContext());

    VkPhysicalDevice physicalDevice = gpu->physicalDevice();
    VkDevice device = gpu->device();

    SkASSERT(gpu);

    if (!gpu->vkCaps().supportsAndroidHWBExternalMemory()) {
        return GrBackendTexture();
    }

    VkFormat format;
    if (!GrBackendFormats::AsVkFormat(backendFormat, &format)) {
        SkDebugf("AsVkFormat failed (valid: %d, backend: %u)",
                 backendFormat.isValid(),
                 (unsigned)backendFormat.backend());
        return GrBackendTexture();
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    if (!skgpu::GetAHardwareBufferProperties(
                &hwbFormatProps, &hwbProps, gpu->vkInterface(), hardwareBuffer, device)) {
        return GrBackendTexture();
    }

    if (hwbFormatProps.format != format) {
        SkDebugf("Queried format not consistent with expected format; got: %d, expected: %d",
                 hwbFormatProps.format,
                 format);
        return GrBackendTexture();
    }

    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.

    const GrVkYcbcrConversionInfo* ycbcrConversion =
            GrBackendFormats::GetVkYcbcrConversionInfo(backendFormat);
    if (!ycbcrConversion) {
        return GrBackendTexture();
    }

    if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
        // TODO: We should not assume the transfer features here and instead should have a way for
        // Ganesh's tracking of intenral images to report whether or not they support transfers.
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(!ycbcrConversion->isValid());
    } else {
        SkASSERT(ycbcrConversion->isValid());
        // We have an external only format
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(format == VK_FORMAT_UNDEFINED);
        SkASSERT(hwbFormatProps.externalFormat == ycbcrConversion->fExternalFormat);
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (format != VK_FORMAT_UNDEFINED) {
        usageFlags = usageFlags |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (isRenderable) {
            usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    // TODO: Check the supported tilings vkGetPhysicalDeviceImageFormatProperties2 to see if we have
    // to use linear. Add better linear support throughout Ganesh.
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    VkImageCreateFlags flags = isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        flags,                                       // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        format,                                      // VkFormat
        { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        tiling,                                      // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        nullptr,                                     // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    VkImage image;
    VkResult err;
    err = VK_CALL(CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (VK_SUCCESS != err) {
        return GrBackendTexture();
    }

    VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    phyDevMemProps.pNext = nullptr;
    VK_CALL(GetPhysicalDeviceMemoryProperties2(physicalDevice, &phyDevMemProps));


    skgpu::VulkanAlloc alloc;
    if (!skgpu::AllocateAndBindImageMemory(&alloc, image, phyDevMemProps, hwbProps, hardwareBuffer,
                                           gpu->vkInterface(), device)) {
        VK_CALL(DestroyImage(device, image, nullptr));
        return GrBackendTexture();
    }

    GrVkImageInfo imageInfo;
    imageInfo.fImage = image;
    imageInfo.fAlloc = alloc;
    imageInfo.fImageTiling = tiling;
    imageInfo.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.fFormat = format;
    imageInfo.fLevelCount = 1;
    // TODO: This should possibly be VK_QUEUE_FAMILY_FOREIGN_EXT but current Adreno devices do not
    // support that extension. Or if we know the source of the AHardwareBuffer is not from a
    // "foreign" device we can leave them as external.
    imageInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    imageInfo.fProtected = isProtectedContent ? GrProtected::kYes : GrProtected::kNo;
    imageInfo.fYcbcrConversionInfo = *ycbcrConversion;
    imageInfo.fSharingMode = imageCreateInfo.sharingMode;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    imageInfo.fPartOfSwapchainOrAndroidWindow = fromAndroidWindow;
#endif

    *deleteProc = delete_vk_image;
    *updateProc = update_vk_image;
    *imageCtx = new VulkanCleanupHelper(gpu, image, alloc.fMemory);

    return GrBackendTextures::MakeVk(width, height, imageInfo);
}

static bool can_import_protected_content(GrDirectContext* dContext) {
    SkASSERT(GrBackendApi::kVulkan == dContext->backend());
    return static_cast<GrVkGpu*>(dContext->priv().getGpu())->protectedContext();
}

GrBackendTexture MakeVulkanBackendTexture(GrDirectContext* dContext,
                                          AHardwareBuffer* hardwareBuffer,
                                          int width, int height,
                                          DeleteImageProc* deleteProc,
                                          UpdateImageProc* updateProc,
                                          TexImageCtx* imageCtx,
                                          bool isProtectedContent,
                                          const GrBackendFormat& backendFormat,
                                          bool isRenderable,
                                          bool fromAndroidWindow) {
    SkASSERT(dContext);
    if (!dContext || dContext->abandoned()) {
        return GrBackendTexture();
    }

    if (GrBackendApi::kVulkan != dContext->backend()) {
        return GrBackendTexture();
    }

    if (isProtectedContent && !can_import_protected_content(dContext)) {
        return GrBackendTexture();
    }

    return make_vk_backend_texture(dContext, hardwareBuffer, width, height, deleteProc,
                                   updateProc, imageCtx, isProtectedContent, backendFormat,
                                   isRenderable, fromAndroidWindow);
}

}  // namespace GrAHardwareBufferUtils

#endif
