
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VulkanWindowContext.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SkAutoMalloc.h"
#include "SkSurface.h"

#include "vk/GrVkExtensions.h"
#include "vk/GrVkImage.h"
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

#define GET_PROC(F) f ## F = (PFN_vk ## F) fGetInstanceProcAddr(fInstance, "vk" #F)
#define GET_DEV_PROC(F) f ## F = (PFN_vk ## F) fGetDeviceProcAddr(fDevice, "vk" #F)

namespace sk_app {

VulkanWindowContext::VulkanWindowContext(const DisplayParams& params,
                                         CreateVkSurfaceFn createVkSurface,
                                         CanPresentFn canPresent,
                                         PFN_vkGetInstanceProcAddr instProc,
                                         PFN_vkGetDeviceProcAddr devProc)
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
    fGetInstanceProcAddr = instProc;
    fGetDeviceProcAddr = devProc;
    this->initializeContext();
}

void VulkanWindowContext::initializeContext() {
    // any config code here (particularly for msaa)?

    PFN_vkGetInstanceProcAddr getInstanceProc = fGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr getDeviceProc = fGetDeviceProcAddr;
    auto getProc = [getInstanceProc, getDeviceProc](const char* proc_name,
                                                    VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return getDeviceProc(device, proc_name);
        }
        return getInstanceProc(instance, proc_name);
    };
    GrVkBackendContext backendContext;
    GrVkExtensions extensions;
    VkPhysicalDeviceFeatures2 features;
    if (!sk_gpu_test::CreateVkBackendContext(getProc, &backendContext, &extensions, &features,
                                             &fDebugCallback, &fPresentQueueIndex, fCanPresentFn)) {
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
        return;
    }

    if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
        !extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
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
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
        return;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    fInterface.reset(new GrVkInterface(backendContext.fGetProc, fInstance, fDevice,
                                       backendContext.fInstanceVersion, physDevVersion,
                                       &extensions));

    GET_PROC(DestroyInstance);
    if (fDebugCallback != VK_NULL_HANDLE) {
        GET_PROC(DestroyDebugReportCallbackEXT);
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

    fContext = GrContext::MakeVulkan(backendContext, fDisplayParams.fGrContextOptions);

    fSurface = fCreateVkSurfaceFn(fInstance);
    if (VK_NULL_HANDLE == fSurface) {
        this->destroyContext();
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
        return;
    }

    VkBool32 supported;
    VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(fPhysicalDevice, fPresentQueueIndex,
                                                       fSurface, &supported);
    if (VK_SUCCESS != res) {
        this->destroyContext();
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
        return;
    }

    if (!this->createSwapchain(-1, -1, fDisplayParams)) {
        this->destroyContext();
        sk_gpu_test::FreeVulkanFeaturesStructs(&features);
        return;
    }

    // create presentQueue
    fGetDeviceQueue(fDevice, fPresentQueueIndex, 0, &fPresentQueue);
    sk_gpu_test::FreeVulkanFeaturesStructs(&features);
}

bool VulkanWindowContext::createSwapchain(int width, int height,
                                          const DisplayParams& params) {
    // check for capabilities
    VkSurfaceCapabilitiesKHR caps;
    VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fSurface, &caps);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t surfaceFormatCount;
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &surfaceFormatCount,
                                              nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
    res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &surfaceFormatCount,
                                              surfaceFormats);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t presentModeCount;
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface, &presentModeCount,
                                                   nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
    VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
    res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface, &presentModeCount,
                                                   presentModes);
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
    fDisplayParams = params;
    fSampleCount = params.fMSAASampleCount;
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
        case VK_FORMAT_B8G8R8A8_SRGB:
            colorType = kBGRA_8888_SkColorType;
            break;
        default:
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

    // destroy the old swapchain
    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
        fDeviceWaitIdle(fDevice);

        this->destroyBuffers();

        fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
    }

    this->createBuffers(swapchainCreateInfo.imageFormat, colorType);

    return true;
}

void VulkanWindowContext::createBuffers(VkFormat format, SkColorType colorType) {
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &fImageCount, nullptr);
    SkASSERT(fImageCount);
    fImages = new VkImage[fImageCount];
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &fImageCount, fImages);

    // set up initial image layouts and create surfaces
    fImageLayouts = new VkImageLayout[fImageCount];
    fSurfaces = new sk_sp<SkSurface>[fImageCount];
    for (uint32_t i = 0; i < fImageCount; ++i) {
        fImageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

        GrVkImageInfo info;
        info.fImage = fImages[i];
        info.fAlloc = GrVkAlloc();
        info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        info.fFormat = format;
        info.fLevelCount = 1;

        GrBackendRenderTarget backendRT(fWidth, fHeight, fSampleCount, info);

        fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(fContext.get(),
                                                              backendRT,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              colorType,
                                                              fDisplayParams.fColorSpace,
                                                              &fDisplayParams.fSurfaceProps);
    }

    // create the command pool for the command buffers
    if (VK_NULL_HANDLE == fCommandPool) {
        VkCommandPoolCreateInfo commandPoolInfo;
        memset(&commandPoolInfo, 0, sizeof(VkCommandPoolCreateInfo));
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // this needs to be on the render queue
        commandPoolInfo.queueFamilyIndex = fGraphicsQueueIndex;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        GR_VK_CALL_ERRCHECK(fInterface,
                            CreateCommandPool(fDevice, &commandPoolInfo,
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
    commandBuffersInfo.commandBufferCount = 1;
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
        GR_VK_CALL_ERRCHECK(fInterface,
                            CreateSemaphore(fDevice, &semaphoreInfo,
                                            nullptr, &fBackbuffers[i].fRenderSemaphore));
        GR_VK_CALL_ERRCHECK(fInterface,
                            AllocateCommandBuffers(fDevice, &commandBuffersInfo,
                                                   &fBackbuffers[i].fTransitionCmdBuffer));
        GR_VK_CALL_ERRCHECK(fInterface,
                            CreateFence(fDevice, &fenceInfo, nullptr,
                                        &fBackbuffers[i].fUsageFence));
    }
    fCurrentBackbufferIndex = fImageCount;
}

void VulkanWindowContext::destroyBuffers() {

    if (fBackbuffers) {
        for (uint32_t i = 0; i < fImageCount + 1; ++i) {
            GR_VK_CALL_ERRCHECK(fInterface,
                                WaitForFences(fDevice, 1,
                                              &fBackbuffers[i].fUsageFence,
                                              true, UINT64_MAX));
            fBackbuffers[i].fImageIndex = -1;
            GR_VK_CALL(fInterface,
                       DestroySemaphore(fDevice,
                                        fBackbuffers[i].fRenderSemaphore,
                                        nullptr));
            GR_VK_CALL(fInterface,
                       FreeCommandBuffers(fDevice, fCommandPool, 1,
                                          &fBackbuffers[i].fTransitionCmdBuffer));
            GR_VK_CALL(fInterface,
                       DestroyFence(fDevice, fBackbuffers[i].fUsageFence, 0));
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
    if (this->isValid()) {
        fQueueWaitIdle(fPresentQueue);
        fDeviceWaitIdle(fDevice);

        this->destroyBuffers();

        if (VK_NULL_HANDLE != fCommandPool) {
            GR_VK_CALL(fInterface, DestroyCommandPool(fDevice, fCommandPool, nullptr));
            fCommandPool = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != fSwapchain) {
            fDestroySwapchainKHR(fDevice, fSwapchain, nullptr);
            fSwapchain = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != fSurface) {
            fDestroySurfaceKHR(fInstance, fSurface, nullptr);
            fSurface = VK_NULL_HANDLE;
        }
    }

    fContext.reset();
    fInterface.reset();

    if (VK_NULL_HANDLE != fDevice) {
        fDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }

#ifdef SK_ENABLE_VK_LAYERS
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugReportCallbackEXT(fInstance, fDebugCallback, nullptr);
    }
#endif

    fPhysicalDevice = VK_NULL_HANDLE;

    if (VK_NULL_HANDLE != fInstance) {
        fDestroyInstance(fInstance, nullptr);
        fInstance = VK_NULL_HANDLE;
    }
}

VulkanWindowContext::BackbufferInfo* VulkanWindowContext::getAvailableBackbuffer() {
    SkASSERT(fBackbuffers);

    ++fCurrentBackbufferIndex;
    if (fCurrentBackbufferIndex > fImageCount) {
        fCurrentBackbufferIndex = 0;
    }

    BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
    GR_VK_CALL_ERRCHECK(fInterface,
                        WaitForFences(fDevice, 1, &backbuffer->fUsageFence,
                                      true, UINT64_MAX));
    return backbuffer;
}

sk_sp<SkSurface> VulkanWindowContext::getBackbufferSurface() {
    BackbufferInfo* backbuffer = this->getAvailableBackbuffer();
    SkASSERT(backbuffer);

    // reset the fence
    GR_VK_CALL_ERRCHECK(fInterface, ResetFences(fDevice, 1, &backbuffer->fUsageFence));
    // semaphores should be in unsignaled state

    VkSemaphoreCreateInfo semaphoreInfo;
    memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;
    VkSemaphore semaphore;
    GR_VK_CALL_ERRCHECK(fInterface, CreateSemaphore(fDevice, &semaphoreInfo,
                                                    nullptr, &semaphore));

    // acquire the image
    VkResult res = fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX,
                                        semaphore, VK_NULL_HANDLE,
                                        &backbuffer->fImageIndex);
    if (VK_ERROR_SURFACE_LOST_KHR == res) {
        // need to figure out how to create a new vkSurface without the platformData*
        // maybe use attach somehow? but need a Window
        GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
        return nullptr;
    }
    if (VK_ERROR_OUT_OF_DATE_KHR == res) {
        // tear swapchain down and try again
        if (!this->createSwapchain(-1, -1, fDisplayParams)) {
            GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
            return nullptr;
        }
        backbuffer = this->getAvailableBackbuffer();
        GR_VK_CALL_ERRCHECK(fInterface,
                            ResetFences(fDevice, 1, &backbuffer->fUsageFence));

        // acquire the image
        res = fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX,
                                   semaphore, VK_NULL_HANDLE,
                                   &backbuffer->fImageIndex);

        if (VK_SUCCESS != res) {
            GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
            return nullptr;
        }
    }

    SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

    GrBackendSemaphore beSemaphore;
    beSemaphore.initVulkan(semaphore);

    surface->wait(1, &beSemaphore);

    return sk_ref_sp(surface);
}

void VulkanWindowContext::swapBuffers() {

    BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
    SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

    GrBackendRenderTarget backendRT = surface->getBackendRenderTarget(
                                                        SkSurface::kFlushRead_BackendHandleAccess);
    GrVkImageInfo imageInfo;
    SkAssertResult(backendRT.getVkImageInfo(&imageInfo));
    // Check to make sure we never change the actually wrapped image
    SkASSERT(imageInfo.fImage == fImages[backbuffer->fImageIndex]);

    VkImageLayout layout = imageInfo.fImageLayout;
    VkPipelineStageFlags srcStageMask = GrVkImage::LayoutToPipelineSrcStageFlags(layout);
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkAccessFlags srcAccessMask = GrVkImage::LayoutToSrcAccessMask(layout);
    VkAccessFlags dstAccessMask = 0;

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // sType
        NULL,                                     // pNext
        srcAccessMask,                            // outputMask
        dstAccessMask,                            // inputMask
        layout,                                   // oldLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // newLayout
        fGraphicsQueueIndex,                      // srcQueueFamilyIndex
        fPresentQueueIndex,                       // dstQueueFamilyIndex
        fImages[backbuffer->fImageIndex],         // image
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } // subresourceRange
    };
    GR_VK_CALL_ERRCHECK(fInterface,
                        ResetCommandBuffer(backbuffer->fTransitionCmdBuffer, 0));
    VkCommandBufferBeginInfo info;
    memset(&info, 0, sizeof(VkCommandBufferBeginInfo));
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = 0;
    GR_VK_CALL_ERRCHECK(fInterface,
                        BeginCommandBuffer(backbuffer->fTransitionCmdBuffer, &info));
    GR_VK_CALL(fInterface,
               CmdPipelineBarrier(backbuffer->fTransitionCmdBuffer,
                                  srcStageMask, dstStageMask, 0,
                                  0, nullptr,
                                  0, nullptr,
                                  1, &imageMemoryBarrier));
    GR_VK_CALL_ERRCHECK(fInterface,
                        EndCommandBuffer(backbuffer->fTransitionCmdBuffer));

    fImageLayouts[backbuffer->fImageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    backendRT.setVkImageLayout(fImageLayouts[backbuffer->fImageIndex]);

    // insert the layout transfer into the queue and wait on the acquire
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &backbuffer->fTransitionCmdBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &backbuffer->fRenderSemaphore;

    GR_VK_CALL_ERRCHECK(fInterface,
                        QueueSubmit(fGraphicsQueue, 1, &submitInfo,
                                    backbuffer->fUsageFence));

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
