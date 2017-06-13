
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SkAutoMalloc.h"
#include "SkSurface.h"
#include "VulkanWindowContext.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkMemory.h"
#include "vk/GrVkUtil.h"
#include "vk/GrVkTypes.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

#define GET_PROC(F) f ## F = (PFN_vk ## F) vkGetInstanceProcAddr(instance, "vk" #F)
#define GET_DEV_PROC(F) f ## F = (PFN_vk ## F) vkGetDeviceProcAddr(device, "vk" #F)

namespace sk_app {

VulkanWindowContext::VulkanWindowContext(const DisplayParams& params,
                                         CreateVkSurfaceFn createVkSurface,
                                         CanPresentFn canPresent)
    : WindowContext(params)
    , fCreateVkSurfaceFn(createVkSurface)
    , fCanPresentFn(canPresent)
    , fSurface(VK_NULL_HANDLE)
    , fSwapchain(VK_NULL_HANDLE)
    , fImages(nullptr)
    , fImageLayouts(nullptr)
    , fSurfaces(nullptr)
    , fCommandPool(VK_NULL_HANDLE)
    , fBackbuffers(nullptr) {
    this->initializeContext();
}

void VulkanWindowContext::initializeContext() {
    // any config code here (particularly for msaa)?
    fBackendContext.reset(GrVkBackendContext::Create(vkGetInstanceProcAddr, vkGetDeviceProcAddr,
                                                     &fPresentQueueIndex, fCanPresentFn));

    if (!(fBackendContext->fExtensions & kKHR_surface_GrVkExtensionFlag) ||
        !(fBackendContext->fExtensions & kKHR_swapchain_GrVkExtensionFlag)) {
        fBackendContext.reset(nullptr);
        return;
    }

    VkInstance instance = fBackendContext->fInstance;
    VkDevice device = fBackendContext->fDevice;
    GET_PROC(DestroySurfaceKHR);
    GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
    GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
    GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    GET_DEV_PROC(CreateSwapchainKHR);
    GET_DEV_PROC(DestroySwapchainKHR);
    GET_DEV_PROC(GetSwapchainImagesKHR);
    GET_DEV_PROC(AcquireNextImageKHR);
    GET_DEV_PROC(QueuePresentKHR);

    fContext = GrContext::Create(kVulkan_GrBackend, (GrBackendContext) fBackendContext.get(),
                                 fDisplayParams.fGrContextOptions);

    fSurface = fCreateVkSurfaceFn(instance);
    if (VK_NULL_HANDLE == fSurface) {
        fBackendContext.reset(nullptr);
        return;
    }

    VkBool32 supported;
    VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(fBackendContext->fPhysicalDevice,
                                                       fPresentQueueIndex, fSurface,
                                                       &supported);
    if (VK_SUCCESS != res) {
        this->destroyContext();
        return;
    }

    if (!this->createSwapchain(-1, -1, fDisplayParams)) {
        this->destroyContext();
        return;
    }

    // create presentQueue
    vkGetDeviceQueue(fBackendContext->fDevice, fPresentQueueIndex, 0, &fPresentQueue);
}

bool VulkanWindowContext::createSwapchain(int width, int height,
                                          const DisplayParams& params) {
    // check for capabilities
    VkSurfaceCapabilitiesKHR caps;
    VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fBackendContext->fPhysicalDevice,
                                                            fSurface, &caps);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t surfaceFormatCount;
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fBackendContext->fPhysicalDevice, fSurface,
                                              &surfaceFormatCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fBackendContext->fPhysicalDevice, fSurface,
                                              &surfaceFormatCount, surfaceFormats);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t presentModeCount;
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fBackendContext->fPhysicalDevice, fSurface,
                                                   &presentModeCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
    VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fBackendContext->fPhysicalDevice, fSurface,
                                                   &presentModeCount, presentModes);
    if (VK_SUCCESS != res) {
        return false;
    }

    VkExtent2D extent = caps.currentExtent;
    // use the hints
    if (extent.width == (uint32_t)-1) {
        extent.width = width;
        extent.height = height;
    }

    // clamp width; to protect us from broken hints
    if (extent.width < caps.minImageExtent.width) {
        extent.width = caps.minImageExtent.width;
    } else if (extent.width > caps.maxImageExtent.width) {
        extent.width = caps.maxImageExtent.width;
    }
    // clamp height
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
    SkASSERT(caps.supportedTransforms & caps.currentTransform);
    SkASSERT(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                                        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR :
                                        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Pick our surface format. For now, just make sure it matches our sRGB request:
    VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    auto srgbColorSpace = SkColorSpace::MakeSRGB();
    bool wantSRGB = srgbColorSpace == params.fColorSpace;
    for (uint32_t i = 0; i < surfaceFormatCount; ++i) {
        GrPixelConfig config = GrVkFormatToPixelConfig(surfaceFormats[i].format);
        if (kUnknown_GrPixelConfig != config &&
            GrPixelConfigIsSRGB(config) == wantSRGB) {
            surfaceFormat = surfaceFormats[i].format;
            colorSpace = surfaceFormats[i].colorSpace;
            break;
        }
    }
    fDisplayParams = params;
    fSampleCount = params.fMSAASampleCount;
    fStencilBits = 8;

    if (VK_FORMAT_UNDEFINED == surfaceFormat) {
        return false;
    }

    // If mailbox mode is available, use it, as it is the lowest-latency non-
    // tearing mode. If not, fall back to FIFO which is always available.
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        // use mailbox
        if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i]) {
            mode = presentModes[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = fSurface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat;
    swapchainCreateInfo.imageColorSpace = colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = usageFlags;

    uint32_t queueFamilies[] = { fBackendContext->fGraphicsQueueIndex, fPresentQueueIndex };
    if (fBackendContext->fGraphicsQueueIndex != fPresentQueueIndex) {
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

    res = fCreateSwapchainKHR(fBackendContext->fDevice, &swapchainCreateInfo, nullptr, &fSwapchain);
    if (VK_SUCCESS != res) {
        return false;
    }

    // destroy the old swapchain
    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
        GR_VK_CALL(fBackendContext->fInterface, DeviceWaitIdle(fBackendContext->fDevice));

        this->destroyBuffers();

        fDestroySwapchainKHR(fBackendContext->fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
    }

    this->createBuffers(swapchainCreateInfo.imageFormat);

    return true;
}

void VulkanWindowContext::createBuffers(VkFormat format) {
    fPixelConfig = GrVkFormatToPixelConfig(format);
    SkASSERT(kUnknown_GrPixelConfig != fPixelConfig);

    fGetSwapchainImagesKHR(fBackendContext->fDevice, fSwapchain, &fImageCount, nullptr);
    SkASSERT(fImageCount);
    fImages = new VkImage[fImageCount];
    fGetSwapchainImagesKHR(fBackendContext->fDevice, fSwapchain, &fImageCount, fImages);

    // set up initial image layouts and create surfaces
    fImageLayouts = new VkImageLayout[fImageCount];
    fSurfaces = new sk_sp<SkSurface>[fImageCount];
    for (uint32_t i = 0; i < fImageCount; ++i) {
        fImageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

        GrVkImageInfo info;
        info.fImage = fImages[i];
        info.fAlloc = { VK_NULL_HANDLE, 0, 0, 0 };
        info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        info.fFormat = format;
        info.fLevelCount = 1;

        GrBackendTexture backendTex(fWidth, fHeight, info);

        fSurfaces[i] = SkSurface::MakeFromBackendTextureAsRenderTarget(fContext, backendTex,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       fSampleCount,
                                                                       fDisplayParams.fColorSpace,
                                                                       &fSurfaceProps);
    }

    // create the command pool for the command buffers
    if (VK_NULL_HANDLE == fCommandPool) {
        VkCommandPoolCreateInfo commandPoolInfo;
        memset(&commandPoolInfo, 0, sizeof(VkCommandPoolCreateInfo));
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // this needs to be on the render queue
        commandPoolInfo.queueFamilyIndex = fBackendContext->fGraphicsQueueIndex;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            CreateCommandPool(fBackendContext->fDevice, &commandPoolInfo,
                                              nullptr, &fCommandPool));
    }

    // set up the backbuffers
    VkSemaphoreCreateInfo semaphoreInfo;
    memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;
    VkCommandBufferAllocateInfo commandBuffersInfo;
    memset(&commandBuffersInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    commandBuffersInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBuffersInfo.pNext = nullptr;
    commandBuffersInfo.commandPool = fCommandPool;
    commandBuffersInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBuffersInfo.commandBufferCount = 2;
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // we create one additional backbuffer structure here, because we want to
    // give the command buffers they contain a chance to finish before we cycle back
    fBackbuffers = new BackbufferInfo[fImageCount + 1];
    for (uint32_t i = 0; i < fImageCount + 1; ++i) {
        fBackbuffers[i].fImageIndex = -1;
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            CreateSemaphore(fBackendContext->fDevice, &semaphoreInfo,
                                            nullptr, &fBackbuffers[i].fAcquireSemaphore));
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            CreateSemaphore(fBackendContext->fDevice, &semaphoreInfo,
                                            nullptr, &fBackbuffers[i].fRenderSemaphore));
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            AllocateCommandBuffers(fBackendContext->fDevice, &commandBuffersInfo,
                                                   fBackbuffers[i].fTransitionCmdBuffers));
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            CreateFence(fBackendContext->fDevice, &fenceInfo, nullptr,
                                        &fBackbuffers[i].fUsageFences[0]));
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            CreateFence(fBackendContext->fDevice, &fenceInfo, nullptr,
                                        &fBackbuffers[i].fUsageFences[1]));
    }
    fCurrentBackbufferIndex = fImageCount;
}

void VulkanWindowContext::destroyBuffers() {

    if (fBackbuffers) {
        for (uint32_t i = 0; i < fImageCount + 1; ++i) {
            GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                                WaitForFences(fBackendContext->fDevice, 2,
                                              fBackbuffers[i].fUsageFences,
                                              true, UINT64_MAX));
            fBackbuffers[i].fImageIndex = -1;
            GR_VK_CALL(fBackendContext->fInterface,
                       DestroySemaphore(fBackendContext->fDevice,
                                        fBackbuffers[i].fAcquireSemaphore,
                                        nullptr));
            GR_VK_CALL(fBackendContext->fInterface,
                       DestroySemaphore(fBackendContext->fDevice,
                                        fBackbuffers[i].fRenderSemaphore,
                                        nullptr));
            GR_VK_CALL(fBackendContext->fInterface,
                       FreeCommandBuffers(fBackendContext->fDevice, fCommandPool, 2,
                                          fBackbuffers[i].fTransitionCmdBuffers));
            GR_VK_CALL(fBackendContext->fInterface,
                       DestroyFence(fBackendContext->fDevice, fBackbuffers[i].fUsageFences[0], 0));
            GR_VK_CALL(fBackendContext->fInterface,
                       DestroyFence(fBackendContext->fDevice, fBackbuffers[i].fUsageFences[1], 0));
        }
    }

    delete[] fBackbuffers;
    fBackbuffers = nullptr;

    // Does this actually free the surfaces?
    delete[] fSurfaces;
    fSurfaces = nullptr;
    delete[] fImageLayouts;
    fImageLayouts = nullptr;
    delete[] fImages;
    fImages = nullptr;
}

VulkanWindowContext::~VulkanWindowContext() {
    this->destroyContext();
}

void VulkanWindowContext::destroyContext() {
    if (!fBackendContext.get()) {
        return;
    }

    GR_VK_CALL(fBackendContext->fInterface, QueueWaitIdle(fPresentQueue));
    GR_VK_CALL(fBackendContext->fInterface, DeviceWaitIdle(fBackendContext->fDevice));

    this->destroyBuffers();

    if (VK_NULL_HANDLE != fCommandPool) {
        GR_VK_CALL(fBackendContext->fInterface, DestroyCommandPool(fBackendContext->fDevice,
                                                                   fCommandPool, nullptr));
        fCommandPool = VK_NULL_HANDLE;
    }

    if (VK_NULL_HANDLE != fSwapchain) {
        fDestroySwapchainKHR(fBackendContext->fDevice, fSwapchain, nullptr);
        fSwapchain = VK_NULL_HANDLE;
    }

    if (VK_NULL_HANDLE != fSurface) {
        fDestroySurfaceKHR(fBackendContext->fInstance, fSurface, nullptr);
        fSurface = VK_NULL_HANDLE;
    }

    fContext->unref();

    fBackendContext.reset(nullptr);
}

VulkanWindowContext::BackbufferInfo* VulkanWindowContext::getAvailableBackbuffer() {
    SkASSERT(fBackbuffers);

    ++fCurrentBackbufferIndex;
    if (fCurrentBackbufferIndex > fImageCount) {
        fCurrentBackbufferIndex = 0;
    }

    BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        WaitForFences(fBackendContext->fDevice, 2, backbuffer->fUsageFences,
                                      true, UINT64_MAX));
    return backbuffer;
}

sk_sp<SkSurface> VulkanWindowContext::getBackbufferSurface() {
    BackbufferInfo* backbuffer = this->getAvailableBackbuffer();
    SkASSERT(backbuffer);

    // reset the fence
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        ResetFences(fBackendContext->fDevice, 2, backbuffer->fUsageFences));
    // semaphores should be in unsignaled state

    // acquire the image
    VkResult res = fAcquireNextImageKHR(fBackendContext->fDevice, fSwapchain, UINT64_MAX,
                                        backbuffer->fAcquireSemaphore, VK_NULL_HANDLE,
                                        &backbuffer->fImageIndex);
    if (VK_ERROR_SURFACE_LOST_KHR == res) {
        // need to figure out how to create a new vkSurface without the platformData*
        // maybe use attach somehow? but need a Window
        return nullptr;
    }
    if (VK_ERROR_OUT_OF_DATE_KHR == res) {
        // tear swapchain down and try again
        if (!this->createSwapchain(-1, -1, fDisplayParams)) {
            return nullptr;
        }
        backbuffer = this->getAvailableBackbuffer();
        GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                            ResetFences(fBackendContext->fDevice, 2, backbuffer->fUsageFences));

        // acquire the image
        res = fAcquireNextImageKHR(fBackendContext->fDevice, fSwapchain, UINT64_MAX,
                                   backbuffer->fAcquireSemaphore, VK_NULL_HANDLE,
                                   &backbuffer->fImageIndex);

        if (VK_SUCCESS != res) {
            return nullptr;
        }
    }

    // set up layout transfer from initial to color attachment
    VkImageLayout layout = fImageLayouts[backbuffer->fImageIndex];
    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED == layout || VK_IMAGE_LAYOUT_PRESENT_SRC_KHR == layout);
    VkPipelineStageFlags srcStageMask = (VK_IMAGE_LAYOUT_UNDEFINED == layout) ?
                                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT :
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags srcAccessMask = (VK_IMAGE_LAYOUT_UNDEFINED == layout) ?
                                  0 : VK_ACCESS_MEMORY_READ_BIT;
    VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // sType
        NULL,                                     // pNext
        srcAccessMask,                            // outputMask
        dstAccessMask,                            // inputMask
        layout,                                   // oldLayout
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // newLayout
        fPresentQueueIndex,                       // srcQueueFamilyIndex
        fBackendContext->fGraphicsQueueIndex,     // dstQueueFamilyIndex
        fImages[backbuffer->fImageIndex],         // image
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } // subresourceRange
    };
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        ResetCommandBuffer(backbuffer->fTransitionCmdBuffers[0], 0));
    VkCommandBufferBeginInfo info;
    memset(&info, 0, sizeof(VkCommandBufferBeginInfo));
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = 0;
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        BeginCommandBuffer(backbuffer->fTransitionCmdBuffers[0], &info));

    GR_VK_CALL(fBackendContext->fInterface,
               CmdPipelineBarrier(backbuffer->fTransitionCmdBuffers[0],
                                  srcStageMask, dstStageMask, 0,
                                  0, nullptr,
                                  0, nullptr,
                                  1, &imageMemoryBarrier));

    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        EndCommandBuffer(backbuffer->fTransitionCmdBuffers[0]));

    VkPipelineStageFlags waitDstStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // insert the layout transfer into the queue and wait on the acquire
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &backbuffer->fAcquireSemaphore;
    submitInfo.pWaitDstStageMask = &waitDstStageFlags;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &backbuffer->fTransitionCmdBuffers[0];
    submitInfo.signalSemaphoreCount = 0;

    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        QueueSubmit(fBackendContext->fQueue, 1, &submitInfo,
                                    backbuffer->fUsageFences[0]));

    GrVkImageInfo* imageInfo;
    SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();
    surface->getRenderTargetHandle((GrBackendObject*)&imageInfo,
                                   SkSurface::kFlushRead_BackendHandleAccess);
    imageInfo->updateImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    return sk_ref_sp(surface);
}

void VulkanWindowContext::swapBuffers() {

    BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
    GrVkImageInfo* imageInfo;
    SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();
    surface->getRenderTargetHandle((GrBackendObject*)&imageInfo,
                                   SkSurface::kFlushRead_BackendHandleAccess);
    // Check to make sure we never change the actually wrapped image
    SkASSERT(imageInfo->fImage == fImages[backbuffer->fImageIndex]);

    VkImageLayout layout = imageInfo->fImageLayout;
    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
    VkAccessFlags dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // sType
        NULL,                                     // pNext
        srcAccessMask,                            // outputMask
        dstAccessMask,                            // inputMask
        layout,                                   // oldLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // newLayout
        fBackendContext->fGraphicsQueueIndex,     // srcQueueFamilyIndex
        fPresentQueueIndex,                       // dstQueueFamilyIndex
        fImages[backbuffer->fImageIndex],         // image
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } // subresourceRange
    };
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        ResetCommandBuffer(backbuffer->fTransitionCmdBuffers[1], 0));
    VkCommandBufferBeginInfo info;
    memset(&info, 0, sizeof(VkCommandBufferBeginInfo));
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = 0;
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        BeginCommandBuffer(backbuffer->fTransitionCmdBuffers[1], &info));
    GR_VK_CALL(fBackendContext->fInterface,
               CmdPipelineBarrier(backbuffer->fTransitionCmdBuffers[1],
                                  srcStageMask, dstStageMask, 0,
                                  0, nullptr,
                                  0, nullptr,
                                  1, &imageMemoryBarrier));
    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        EndCommandBuffer(backbuffer->fTransitionCmdBuffers[1]));

    fImageLayouts[backbuffer->fImageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // insert the layout transfer into the queue and wait on the acquire
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &backbuffer->fTransitionCmdBuffers[1];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &backbuffer->fRenderSemaphore;

    GR_VK_CALL_ERRCHECK(fBackendContext->fInterface,
                        QueueSubmit(fBackendContext->fQueue, 1, &submitInfo,
                                    backbuffer->fUsageFences[1]));

    // Submit present operation to present queue
    const VkPresentInfoKHR presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
        NULL, // pNext
        1, // waitSemaphoreCount
        &backbuffer->fRenderSemaphore, // pWaitSemaphores
        1, // swapchainCount
        &fSwapchain, // pSwapchains
        &backbuffer->fImageIndex, // pImageIndices
        NULL // pResults
    };

    fQueuePresentKHR(fPresentQueue, &presentInfo);
}

}   //namespace sk_app
