/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/VulkanWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSemaphore.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkTypes.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/gpu/GpuTypesPriv.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/base/SkAutoMalloc.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/vulkanmemoryallocator/VulkanAMDMemoryAllocator.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

#define GET_PROC(F) f ## F = \
    (PFN_vk ## F) backendContext.fGetProc("vk" #F, fInstance, VK_NULL_HANDLE)
#define GET_DEV_PROC(F) f ## F = \
    (PFN_vk ## F) backendContext.fGetProc("vk" #F, VK_NULL_HANDLE, fDevice)

namespace skwindow::internal {

VulkanWindowContext::VulkanWindowContext(std::unique_ptr<const DisplayParams> params,
                                         CreateVkSurfaceFn createVkSurface,
                                         CanPresentFn canPresent,
                                         PFN_vkGetInstanceProcAddr instProc)
        : WindowContext(std::move(params))
        , fDeviceSurface(VK_NULL_HANDLE)
        , fSwapchain(VK_NULL_HANDLE)
        , fCreateVkSurfaceFn(std::move(createVkSurface))
        , fCanPresentFn(std::move(canPresent)) {
    fGetInstanceProcAddr = instProc;
    this->initializeContext();
}

void VulkanWindowContext::initializeContext() {
    SkASSERT(!fContext);
    // Any config code here (particularly for msaa)?

    PFN_vkGetInstanceProcAddr getInstanceProc = fGetInstanceProcAddr;
    skgpu::VulkanBackendContext backendContext;
    skgpu::VulkanExtensions extensions;
    sk_gpu_test::TestVkFeatures features;
    if (!sk_gpu_test::CreateVkBackendContext(getInstanceProc,
                                             &backendContext,
                                             &extensions,
                                             &features,
                                             &fDebugMessenger,
                                             &fPresentQueueIndex,
                                             fCanPresentFn,
                                             fDisplayParams->createProtectedNativeBackend())) {
        return;
    }

    if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
        !extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
        return;
    }

    fInstance = backendContext.fInstance;
    fPhysicalDevice = backendContext.fPhysicalDevice;
    fDevice = backendContext.fDevice;
    fGraphicsQueueIndex = backendContext.fGraphicsQueueIndex;
    fGraphicsQueue = backendContext.fQueue;

    PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                    backendContext.fGetProc("vkGetPhysicalDeviceProperties",
                                            backendContext.fInstance,
                                            VK_NULL_HANDLE));
    if (!localGetPhysicalDeviceProperties) {
        return;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    fInterface.reset(new skgpu::VulkanInterface(backendContext.fGetProc,
                                                fInstance,
                                                fDevice,
                                                backendContext.fMaxAPIVersion,
                                                physDevVersion,
                                                &extensions));

    GET_PROC(DestroyInstance);
    if (fDebugMessenger != VK_NULL_HANDLE) {
        GET_PROC(DestroyDebugUtilsMessengerEXT);
    }
    GET_PROC(DestroySurfaceKHR);
    GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
    GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
    GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    GET_DEV_PROC(DeviceWaitIdle);
    GET_DEV_PROC(QueueWaitIdle);
    GET_DEV_PROC(DestroyDevice);
    GET_DEV_PROC(CreateSwapchainKHR);
    GET_DEV_PROC(DestroySwapchainKHR);
    GET_DEV_PROC(GetSwapchainImagesKHR);
    GET_DEV_PROC(AcquireNextImageKHR);
    GET_DEV_PROC(QueuePresentKHR);
    GET_DEV_PROC(GetDeviceQueue);

    backendContext.fMemoryAllocator =
            skgpu::VulkanAMDMemoryAllocator::Make(fInstance,
                                                  backendContext.fPhysicalDevice,
                                                  backendContext.fDevice,
                                                  physDevVersion,
                                                  &extensions,
                                                  fInterface.get(),
                                                  skgpu::ThreadSafe::kNo);

    fContext = GrDirectContexts::MakeVulkan(backendContext, fDisplayParams->grContextOptions());

    fDeviceSurface = fCreateVkSurfaceFn(fInstance);
    if (VK_NULL_HANDLE == fDeviceSurface) {
        this->destroyContext();
        return;
    }

    VkBool32 supported;
    VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(
            fPhysicalDevice, fPresentQueueIndex, fDeviceSurface, &supported);
    if (VK_SUCCESS != res) {
        this->destroyContext();
        return;
    }

    if (!this->createSwapchain(-1, -1)) {
        this->destroyContext();
        return;
    }

    fGetDeviceQueue(fDevice, fPresentQueueIndex, /*queueIndex=*/0, &fPresentQueue);
}

bool VulkanWindowContext::createSwapchain(int width, int height) {
    // Check surface capabilities
    VkSurfaceCapabilitiesKHR caps;
    VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fDeviceSurface, &caps);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t surfaceFormatCount;
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fDeviceSurface, &surfaceFormatCount,
                                              nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fDeviceSurface, &surfaceFormatCount,
                                              surfaceFormats);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t presentModeCount;
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fDeviceSurface, &presentModeCount,
                                                   nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
    VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fDeviceSurface, &presentModeCount,
                                                   presentModes);
    if (VK_SUCCESS != res) {
        return false;
    }

    VkExtent2D extent = caps.currentExtent;
    // Use the hints for width + height
    if (extent.width == (uint32_t)-1) {
        extent.width = width;
        extent.height = height;
    }
    // Clamp the values to protect from broken hints
    if (extent.width < caps.minImageExtent.width) {
        extent.width = caps.minImageExtent.width;
    } else if (extent.width > caps.maxImageExtent.width) {
        extent.width = caps.maxImageExtent.width;
    }
    if (extent.height < caps.minImageExtent.height) {
        extent.height = caps.minImageExtent.height;
    } else if (extent.height > caps.maxImageExtent.height) {
        extent.height = caps.maxImageExtent.height;
    }
    fWidth = (int)extent.width;
    fHeight = (int)extent.height;

    uint32_t imageCount = caps.minImageCount + 2;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        // Application must settle for fewer images than desired:
        imageCount = caps.maxImageCount;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    SkASSERT((caps.supportedUsageFlags & usageFlags) == usageFlags);
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    SkASSERT(caps.supportedTransforms & caps.currentTransform);
    SkASSERT(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                                        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR :
                                        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Pick our surface format.
    VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    for (uint32_t i = 0; i < surfaceFormatCount; ++i) {
        VkFormat localFormat = surfaceFormats[i].format;
        if (GrVkFormatIsSupported(localFormat)) {
            surfaceFormat = localFormat;
            colorSpace = surfaceFormats[i].colorSpace;
            break;
        }
    }
    fSampleCount = std::max(1, fDisplayParams->msaaSampleCount());
    fStencilBits = 8;

    if (VK_FORMAT_UNDEFINED == surfaceFormat) {
        return false;
    }

    SkColorType colorType;
    switch (surfaceFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM: // fall through
        case VK_FORMAT_R8G8B8A8_SRGB:
            colorType = kRGBA_8888_SkColorType;
            break;
        case VK_FORMAT_B8G8R8A8_UNORM: // fall through
            colorType = kBGRA_8888_SkColorType;
            break;
        default:
            return false;
    }

    // If mailbox mode is available, use it, as it is the lowest-latency non-
    // tearing mode. If not, fall back to FIFO which is always available.
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    bool hasImmediate = false;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        // use mailbox
        if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i]) {
            mode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        if (VK_PRESENT_MODE_IMMEDIATE_KHR == presentModes[i]) {
            hasImmediate = true;
        }
    }
    if (fDisplayParams->disableVsync() && hasImmediate) {
        mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.flags = fDisplayParams->createProtectedNativeBackend()
                                        ? VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR
                                        : 0;
    swapchainCreateInfo.surface = fDeviceSurface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat;
    swapchainCreateInfo.imageColorSpace = colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = usageFlags;

    uint32_t queueFamilies[] = { fGraphicsQueueIndex, fPresentQueueIndex };
    if (fGraphicsQueueIndex != fPresentQueueIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = composite_alpha;
    swapchainCreateInfo.presentMode = mode;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = fSwapchain;

    res = fCreateSwapchainKHR(fDevice, &swapchainCreateInfo, nullptr, &fSwapchain);
    if (VK_SUCCESS != res) {
        return false;
    }

    // Destroy the old swapchain
    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
        fDeviceWaitIdle(fDevice);
        this->resetSwapchainImages();
        fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    // If buffer creation fails, destroy the swapchain.
    if (!this->populateSwapchainImages(swapchainCreateInfo.imageFormat,
                                       usageFlags,
                                       colorType,
                                       swapchainCreateInfo.imageSharingMode)) {
        fDeviceWaitIdle(fDevice);
        this->resetSwapchainImages();
        fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool VulkanWindowContext::populateSwapchainImages(VkFormat format,
                                                  VkImageUsageFlags usageFlags,
                                                  SkColorType colorType,
                                                  VkSharingMode sharingMode) {
    // Determine number of swapchain images
    uint32_t swapchainImgCount;
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &swapchainImgCount, /*pSwapchainImages*/nullptr);
    SkASSERT(swapchainImgCount);

    // Define an array of VkImages and query the driver to populate it
    skia_private::AutoTArray<VkImage> vkImages((size_t)swapchainImgCount);
    std::fill_n(vkImages.get(), swapchainImgCount, VK_NULL_HANDLE);
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &swapchainImgCount, vkImages.get());

    // Populate all swapchain image representations
    fImages = skia_private::AutoTArray<SwapchainImage>((size_t)swapchainImgCount);
    for (uint32_t i = 0; i < swapchainImgCount; ++i) {
        // Make sure we were provided a valid VkImage handle
        SkASSERT(vkImages[i] != VK_NULL_HANDLE);
        fImages[i].fVkImage = vkImages[i];

        // Create the semaphore that will be signaled once the image done being rendered
        static const VkSemaphoreCreateInfo submitSemInfo =
                {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, /*pNext=*/nullptr, /*flags=*/0};
        SkDEBUGCODE(VkResult result = )GR_VK_CALL(
                fInterface,
                CreateSemaphore(fDevice,
                                &submitSemInfo,
                                /*pAllocator=*/nullptr,
                                &fImages[i].fRenderCompletionSemaphore));
        SkASSERT(result == VK_SUCCESS && fImages[i].fRenderCompletionSemaphore != VK_NULL_HANDLE);

        // Populate GrVkImageInfo for Surface creation
        GrVkImageInfo info;
        info.fImage = vkImages[i];
        info.fAlloc = skgpu::VulkanAlloc();
        info.fFormat = format;
        info.fImageUsageFlags = usageFlags;
        info.fLevelCount = 1;
        info.fCurrentQueueFamily = fPresentQueueIndex;
        info.fProtected = skgpu::Protected(fDisplayParams->createProtectedNativeBackend());
        info.fSharingMode = sharingMode;

        // Based on whether the image can be sampled, create a Surface to present each image
        if (usageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
            GrBackendTexture backendTexture = GrBackendTextures::MakeVk(fWidth, fHeight, info);
            fImages[i].fSurface =
                    SkSurfaces::WrapBackendTexture(fContext.get(),
                                                   backendTexture,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   fDisplayParams->msaaSampleCount(),
                                                   colorType,
                                                   fDisplayParams->colorSpace(),
                                                   &fDisplayParams->surfaceProps());
        } else {
            if (fDisplayParams->msaaSampleCount() > 1) {
                return false;
            }
            info.fSampleCount = fSampleCount;
            GrBackendRenderTarget backendRT = GrBackendRenderTargets::MakeVk(fWidth, fHeight, info);
            fImages[i].fSurface =
                    SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                        backendRT,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        colorType,
                                                        fDisplayParams->colorSpace(),
                                                        &fDisplayParams->surfaceProps());
        }

        // If surface creation was unsuccessful, return false to indicate failure
        if (!fImages[i].fSurface) {
            // Clean up any previously-created semaphores before returning
            this->resetSwapchainImages();
            return false;
        }
    }
    return true;
}

void VulkanWindowContext::submitToGpu() {
    SkSurface* surface = fImages[fCurrentImageIndex].fSurface.get();

    GrBackendSemaphore backendRenderSemaphore =
            GrBackendSemaphores::MakeVk(fImages[fCurrentImageIndex].fRenderCompletionSemaphore);

    GrFlushInfo info;
    info.fNumSemaphores = 1;
    info.fSignalSemaphores = &backendRenderSemaphore;
    skgpu::MutableTextureState presentState = skgpu::MutableTextureStates::MakeVulkan(
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, fPresentQueueIndex);
    auto dContext = surface->recordingContext()->asDirectContext();
    dContext->flush(surface, info, &presentState);
    dContext->submit();
}

void VulkanWindowContext::resetSwapchainImages() {
    if (fImages.empty()) {
        return;
    }

    // Clean up the image's semaphores
    for (size_t i = 0; i < fImages.size(); i++) {
        if (fImages[i].fRenderCompletionSemaphore != VK_NULL_HANDLE) {
            GR_VK_CALL(fInterface, DestroySemaphore(fDevice,
                                                    fImages[i].fRenderCompletionSemaphore,
                                                    /*pAllocator=*/nullptr));
        }
    }

    fImages.reset();
}

VulkanWindowContext::~VulkanWindowContext() {
    this->destroyContext();
}

void VulkanWindowContext::destroyContext() {
    if (this->isValid()) {
        fQueueWaitIdle(fPresentQueue);
        fDeviceWaitIdle(fDevice);

        this->resetSwapchainImages();

        if (VK_NULL_HANDLE != fSwapchain) {
            fDestroySwapchainKHR(fDevice, fSwapchain, nullptr);
            fSwapchain = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != fDeviceSurface) {
            fDestroySurfaceKHR(fInstance, fDeviceSurface, nullptr);
            fDeviceSurface = VK_NULL_HANDLE;
        }
    }

    SkASSERT(fContext->unique());
    fContext.reset();
    fInterface.reset();

    if (VK_NULL_HANDLE != fDevice) {
        fDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }

#ifdef SK_ENABLE_VK_LAYERS
    if (fDebugMessenger != VK_NULL_HANDLE) {
        fDestroyDebugUtilsMessengerEXT(fInstance, fDebugMessenger, nullptr);
    }
#endif

    fPhysicalDevice = VK_NULL_HANDLE;

    if (VK_NULL_HANDLE != fInstance) {
        fDestroyInstance(fInstance, nullptr);
        fInstance = VK_NULL_HANDLE;
    }
}

sk_sp<SkSurface> VulkanWindowContext::getBackbufferSurface() {
    // Create a new, unsignaled semaphore to be signaled upon swapchain image acquisition.
    static const VkSemaphoreCreateInfo acquireSemInfo =
            { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, /*pNext=*/nullptr, /*flags=*/0 };
    SkDEBUGCODE(VkResult result = )GR_VK_CALL(
            fInterface,
            CreateSemaphore(fDevice, &acquireSemInfo, /*pAllocator=*/nullptr, &fAcquireSemaphore));
    SkASSERT(result == VK_SUCCESS && fAcquireSemaphore != VK_NULL_HANDLE);

    // Acquire the next available presentable image's index.
    VkResult res = fAcquireNextImageKHR(fDevice,
                                        fSwapchain,
                                        /*timeout=*/UINT64_MAX,
                                        fAcquireSemaphore,
                                        /*VkFence=*/VK_NULL_HANDLE,
                                        &fCurrentImageIndex);
    if (VK_ERROR_SURFACE_LOST_KHR == res) {
        // TODO: Recreate fDeviceSurface using fCreateVkSurfaceFn, and then rebuild the swapchain
        GR_VK_CALL(fInterface,
                   DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
        return nullptr;
    }
    if (VK_ERROR_OUT_OF_DATE_KHR == res) {
        if (!this->createSwapchain(-1, -1)) {
            GR_VK_CALL(fInterface,
                       DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
            return nullptr;
        }

        res = fAcquireNextImageKHR(fDevice,
                                   fSwapchain,
                                   UINT64_MAX,
                                   fAcquireSemaphore,
                                   /*VkFence=*/VK_NULL_HANDLE,
                                   &fCurrentImageIndex);
        if (VK_SUCCESS != res) {
            GR_VK_CALL(fInterface,
                       DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
            return nullptr;
        }
    }

    // Set up the SkSurface associated with the current backbuffer's image and return it.
    SkSurface* surface = fImages[fCurrentImageIndex].fSurface.get();
    GrBackendSemaphore beSemaphore = GrBackendSemaphores::MakeVk(fAcquireSemaphore);
    surface->wait(/*numSemaphores=*/1, &beSemaphore);

    return sk_ref_sp(surface);
}

void VulkanWindowContext::onSwapBuffers() {
    // Submit the GPU work associated with the current backbuffer
    this->submitToGpu();

    // Submit present operation to present queue
    auto renderCompletionSemaphore = &fImages[fCurrentImageIndex].fRenderCompletionSemaphore;
    const VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
                                          nullptr,                            // pNext
                                          1,                                  // waitSemaphoreCount
                                          renderCompletionSemaphore,          // pWaitSemaphores
                                          1,                                  // swapchainCount
                                          &fSwapchain,                        // pSwapchains
                                          &fCurrentImageIndex,                // pImageIndices
                                          nullptr};                           // pResults
    fQueuePresentKHR(fPresentQueue, &presentInfo);
}

}  // namespace skwindow::internal
