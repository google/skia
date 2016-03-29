/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkUtil.h"

GrVkInterface::GrVkInterface() {
}

#define GET_PROC(F) functions->f ## F = (PFN_vk ## F) vkGetInstanceProcAddr(instance, "vk" #F)
#define GET_PROC_LOCAL(inst, F) PFN_vk ## F F = (PFN_vk ## F) vkGetInstanceProcAddr(inst, "vk" #F)
#define GET_DEV_PROC(F) functions->f ## F = (PFN_vk ## F) vkGetDeviceProcAddr(device, "vk" #F)

const GrVkInterface* GrVkCreateInterface(VkInstance instance, VkDevice device,
                                         uint32_t extensionFlags) {

    GrVkInterface* interface = new GrVkInterface();
    GrVkInterface::Functions* functions = &interface->fFunctions;

    GET_PROC(CreateInstance);
    GET_PROC(DestroyInstance);
    GET_PROC(EnumeratePhysicalDevices);
    GET_PROC(GetPhysicalDeviceFeatures);
    GET_PROC(GetPhysicalDeviceFormatProperties);
    GET_PROC(GetPhysicalDeviceImageFormatProperties);
    GET_PROC(GetPhysicalDeviceProperties);
    GET_PROC(GetPhysicalDeviceQueueFamilyProperties);
    GET_PROC(GetPhysicalDeviceMemoryProperties);
    GET_PROC(CreateDevice);
    GET_PROC(DestroyDevice);
    GET_PROC(EnumerateInstanceExtensionProperties);
    GET_PROC(EnumerateDeviceExtensionProperties);
    GET_PROC(EnumerateInstanceLayerProperties);
    GET_PROC(EnumerateDeviceLayerProperties);
    GET_DEV_PROC(GetDeviceQueue);
    GET_DEV_PROC(QueueSubmit);
    GET_DEV_PROC(QueueWaitIdle);
    GET_DEV_PROC(DeviceWaitIdle);
    GET_DEV_PROC(AllocateMemory);
    GET_DEV_PROC(FreeMemory);
    GET_DEV_PROC(MapMemory);
    GET_DEV_PROC(UnmapMemory);
    GET_DEV_PROC(FlushMappedMemoryRanges);
    GET_DEV_PROC(InvalidateMappedMemoryRanges);
    GET_DEV_PROC(GetDeviceMemoryCommitment);
    GET_DEV_PROC(BindBufferMemory);
    GET_DEV_PROC(BindImageMemory);
    GET_DEV_PROC(GetBufferMemoryRequirements);
    GET_DEV_PROC(GetImageMemoryRequirements);
    GET_DEV_PROC(GetImageSparseMemoryRequirements);
    GET_PROC(GetPhysicalDeviceSparseImageFormatProperties);
    GET_DEV_PROC(QueueBindSparse);
    GET_DEV_PROC(CreateFence);
    GET_DEV_PROC(DestroyFence);
    GET_DEV_PROC(ResetFences);
    GET_DEV_PROC(GetFenceStatus);
    GET_DEV_PROC(WaitForFences);
    GET_DEV_PROC(CreateSemaphore);
    GET_DEV_PROC(DestroySemaphore);
    GET_DEV_PROC(CreateEvent);
    GET_DEV_PROC(DestroyEvent);
    GET_DEV_PROC(GetEventStatus);
    GET_DEV_PROC(SetEvent);
    GET_DEV_PROC(ResetEvent);
    GET_DEV_PROC(CreateQueryPool);
    GET_DEV_PROC(DestroyQueryPool);
    GET_DEV_PROC(GetQueryPoolResults);
    GET_DEV_PROC(CreateBuffer);
    GET_DEV_PROC(DestroyBuffer);
    GET_DEV_PROC(CreateBufferView);
    GET_DEV_PROC(DestroyBufferView);
    GET_DEV_PROC(CreateImage);
    GET_DEV_PROC(DestroyImage);
    GET_DEV_PROC(GetImageSubresourceLayout);
    GET_DEV_PROC(CreateImageView);
    GET_DEV_PROC(DestroyImageView);
    GET_DEV_PROC(CreateShaderModule);
    GET_DEV_PROC(DestroyShaderModule);
    GET_DEV_PROC(CreatePipelineCache);
    GET_DEV_PROC(DestroyPipelineCache);
    GET_DEV_PROC(GetPipelineCacheData);
    GET_DEV_PROC(MergePipelineCaches);
    GET_DEV_PROC(CreateGraphicsPipelines);
    GET_DEV_PROC(CreateComputePipelines);
    GET_DEV_PROC(DestroyPipeline);
    GET_DEV_PROC(CreatePipelineLayout);
    GET_DEV_PROC(DestroyPipelineLayout);
    GET_DEV_PROC(CreateSampler);
    GET_DEV_PROC(DestroySampler);
    GET_DEV_PROC(CreateDescriptorSetLayout);
    GET_DEV_PROC(DestroyDescriptorSetLayout);
    GET_DEV_PROC(CreateDescriptorPool);
    GET_DEV_PROC(DestroyDescriptorPool);
    GET_DEV_PROC(ResetDescriptorPool);
    GET_DEV_PROC(AllocateDescriptorSets);
    GET_DEV_PROC(FreeDescriptorSets);
    GET_DEV_PROC(UpdateDescriptorSets);
    GET_DEV_PROC(CreateFramebuffer);
    GET_DEV_PROC(DestroyFramebuffer);
    GET_DEV_PROC(CreateRenderPass);
    GET_DEV_PROC(DestroyRenderPass);
    GET_DEV_PROC(GetRenderAreaGranularity);
    GET_DEV_PROC(CreateCommandPool);
    GET_DEV_PROC(DestroyCommandPool);
    GET_DEV_PROC(ResetCommandPool);
    GET_DEV_PROC(AllocateCommandBuffers);
    GET_DEV_PROC(FreeCommandBuffers);
    GET_DEV_PROC(BeginCommandBuffer);
    GET_DEV_PROC(EndCommandBuffer);
    GET_DEV_PROC(ResetCommandBuffer);
    GET_DEV_PROC(CmdBindPipeline);
    GET_DEV_PROC(CmdSetViewport);
    GET_DEV_PROC(CmdSetScissor);
    GET_DEV_PROC(CmdSetLineWidth);
    GET_DEV_PROC(CmdSetDepthBias);
    GET_DEV_PROC(CmdSetBlendConstants);
    GET_DEV_PROC(CmdSetDepthBounds);
    GET_DEV_PROC(CmdSetStencilCompareMask);
    GET_DEV_PROC(CmdSetStencilWriteMask);
    GET_DEV_PROC(CmdSetStencilReference);
    GET_DEV_PROC(CmdBindDescriptorSets);
    GET_DEV_PROC(CmdBindIndexBuffer);
    GET_DEV_PROC(CmdBindVertexBuffers);
    GET_DEV_PROC(CmdDraw);
    GET_DEV_PROC(CmdDrawIndexed);
    GET_DEV_PROC(CmdDrawIndirect);
    GET_DEV_PROC(CmdDrawIndexedIndirect);
    GET_DEV_PROC(CmdDispatch);
    GET_DEV_PROC(CmdDispatchIndirect);
    GET_DEV_PROC(CmdCopyBuffer);
    GET_DEV_PROC(CmdCopyImage);
    GET_DEV_PROC(CmdBlitImage);
    GET_DEV_PROC(CmdCopyBufferToImage);
    GET_DEV_PROC(CmdCopyImageToBuffer);
    GET_DEV_PROC(CmdUpdateBuffer);
    GET_DEV_PROC(CmdFillBuffer);
    GET_DEV_PROC(CmdClearColorImage);
    GET_DEV_PROC(CmdClearDepthStencilImage);
    GET_DEV_PROC(CmdClearAttachments);
    GET_DEV_PROC(CmdResolveImage);
    GET_DEV_PROC(CmdSetEvent);
    GET_DEV_PROC(CmdResetEvent);
    GET_DEV_PROC(CmdWaitEvents);
    GET_DEV_PROC(CmdPipelineBarrier);
    GET_DEV_PROC(CmdBeginQuery);
    GET_DEV_PROC(CmdEndQuery);
    GET_DEV_PROC(CmdResetQueryPool);
    GET_DEV_PROC(CmdWriteTimestamp);
    GET_DEV_PROC(CmdCopyQueryPoolResults);
    GET_DEV_PROC(CmdPushConstants);
    GET_DEV_PROC(CmdBeginRenderPass);
    GET_DEV_PROC(CmdNextSubpass);
    GET_DEV_PROC(CmdEndRenderPass);
    GET_DEV_PROC(CmdExecuteCommands);
    if (extensionFlags & kKHR_surface_GrVkExtensionFlag) {
        GET_PROC(DestroySurfaceKHR);
        GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
        GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
        GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    }
    if (extensionFlags & kKHR_surface_GrVkExtensionFlag) {
        GET_DEV_PROC(CreateSwapchainKHR);
        GET_DEV_PROC(DestroySwapchainKHR);
        GET_DEV_PROC(GetSwapchainImagesKHR);
        GET_DEV_PROC(AcquireNextImageKHR);
        GET_DEV_PROC(QueuePresentKHR);
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (extensionFlags & kKHR_win32_surface_GrVkExtensionFlag) {
        GET_PROC(CreateWin32SurfaceKHR);
        GET_PROC(GetPhysicalDeviceWin32PresentationSupportKHR);
    }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (extensionFlags & kKHR_android_surface_GrVkExtensionFlag) {
        GET_PROC(CreateAndroidSurfaceKHR);
    }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    if (extensionFlags & kKHR_xlib_surface_GrVkExtensionFlag) {
        GET_PROC(CreateXlibSurfaceKHR);
        GET_PROC(GetPhysicalDeviceXlibPresentationSupportKHR);
    }
#endif

    // We probably don't care about these, they're for consoles
    //GET_PROC(GetPhysicalDeviceDisplayPropertiesKHR);
    //GET_PROC(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    //GET_PROC(GetDisplayPlaneSupportedDisplaysKHR);
    //GET_PROC(GetDisplayModePropertiesKHR);
    //GET_PROC(CreateDisplayModeKHR);
    //GET_PROC(GetDisplayPlaneCapabilitiesKHR);
    //GET_PROC(CreateDisplayPlaneSurfaceKHR);
    //GET_DEV_PROC(CreateSharedSwapchainsKHR);

    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        GET_PROC(CreateDebugReportCallbackEXT);
        GET_PROC(DebugReportMessageEXT);
        GET_PROC(DestroyDebugReportCallbackEXT);
    }

    return interface;
}

#define RETURN_FALSE_INTERFACE                                                                   \
    if (kIsDebug) { SkDebugf("%s:%d GrVkInterface::validate() failed.\n", __FILE__, __LINE__); } \
    return false;

bool GrVkInterface::validate() const {
    // functions that are always required
    if (NULL == fFunctions.fCreateInstance ||
        NULL == fFunctions.fDestroyInstance ||
        NULL == fFunctions.fEnumeratePhysicalDevices ||
        NULL == fFunctions.fGetPhysicalDeviceFeatures ||
        NULL == fFunctions.fGetPhysicalDeviceFormatProperties ||
        NULL == fFunctions.fGetPhysicalDeviceImageFormatProperties ||
        NULL == fFunctions.fGetPhysicalDeviceProperties ||
        NULL == fFunctions.fGetPhysicalDeviceQueueFamilyProperties ||
        NULL == fFunctions.fGetPhysicalDeviceMemoryProperties ||
        NULL == fFunctions.fCreateDevice ||
        NULL == fFunctions.fDestroyDevice ||
        NULL == fFunctions.fEnumerateInstanceExtensionProperties ||
        NULL == fFunctions.fEnumerateDeviceExtensionProperties ||
        NULL == fFunctions.fEnumerateInstanceLayerProperties ||
        NULL == fFunctions.fEnumerateDeviceLayerProperties ||
        NULL == fFunctions.fGetDeviceQueue ||
        NULL == fFunctions.fQueueSubmit ||
        NULL == fFunctions.fQueueWaitIdle ||
        NULL == fFunctions.fDeviceWaitIdle ||
        NULL == fFunctions.fAllocateMemory ||
        NULL == fFunctions.fFreeMemory ||
        NULL == fFunctions.fMapMemory ||
        NULL == fFunctions.fUnmapMemory ||
        NULL == fFunctions.fFlushMappedMemoryRanges ||
        NULL == fFunctions.fInvalidateMappedMemoryRanges ||
        NULL == fFunctions.fGetDeviceMemoryCommitment ||
        NULL == fFunctions.fBindBufferMemory ||
        NULL == fFunctions.fBindImageMemory ||
        NULL == fFunctions.fGetBufferMemoryRequirements ||
        NULL == fFunctions.fGetImageMemoryRequirements ||
        NULL == fFunctions.fGetImageSparseMemoryRequirements ||
        NULL == fFunctions.fGetPhysicalDeviceSparseImageFormatProperties ||
        NULL == fFunctions.fQueueBindSparse ||
        NULL == fFunctions.fCreateFence ||
        NULL == fFunctions.fDestroyFence ||
        NULL == fFunctions.fResetFences ||
        NULL == fFunctions.fGetFenceStatus ||
        NULL == fFunctions.fWaitForFences ||
        NULL == fFunctions.fCreateSemaphore ||
        NULL == fFunctions.fDestroySemaphore ||
        NULL == fFunctions.fCreateEvent ||
        NULL == fFunctions.fDestroyEvent ||
        NULL == fFunctions.fGetEventStatus ||
        NULL == fFunctions.fSetEvent ||
        NULL == fFunctions.fResetEvent ||
        NULL == fFunctions.fCreateQueryPool ||
        NULL == fFunctions.fDestroyQueryPool ||
        NULL == fFunctions.fGetQueryPoolResults ||
        NULL == fFunctions.fCreateBuffer ||
        NULL == fFunctions.fDestroyBuffer ||
        NULL == fFunctions.fCreateBufferView ||
        NULL == fFunctions.fDestroyBufferView ||
        NULL == fFunctions.fCreateImage ||
        NULL == fFunctions.fDestroyImage ||
        NULL == fFunctions.fGetImageSubresourceLayout ||
        NULL == fFunctions.fCreateImageView ||
        NULL == fFunctions.fDestroyImageView ||
        NULL == fFunctions.fCreateShaderModule ||
        NULL == fFunctions.fDestroyShaderModule ||
        NULL == fFunctions.fCreatePipelineCache ||
        NULL == fFunctions.fDestroyPipelineCache ||
        NULL == fFunctions.fGetPipelineCacheData ||
        NULL == fFunctions.fMergePipelineCaches ||
        NULL == fFunctions.fCreateGraphicsPipelines ||
        NULL == fFunctions.fCreateComputePipelines ||
        NULL == fFunctions.fDestroyPipeline ||
        NULL == fFunctions.fCreatePipelineLayout ||
        NULL == fFunctions.fDestroyPipelineLayout ||
        NULL == fFunctions.fCreateSampler ||
        NULL == fFunctions.fDestroySampler ||
        NULL == fFunctions.fCreateDescriptorSetLayout ||
        NULL == fFunctions.fDestroyDescriptorSetLayout ||
        NULL == fFunctions.fCreateDescriptorPool ||
        NULL == fFunctions.fDestroyDescriptorPool ||
        NULL == fFunctions.fResetDescriptorPool ||
        NULL == fFunctions.fAllocateDescriptorSets ||
        NULL == fFunctions.fFreeDescriptorSets ||
        NULL == fFunctions.fUpdateDescriptorSets ||
        NULL == fFunctions.fCreateFramebuffer ||
        NULL == fFunctions.fDestroyFramebuffer ||
        NULL == fFunctions.fCreateRenderPass ||
        NULL == fFunctions.fDestroyRenderPass ||
        NULL == fFunctions.fGetRenderAreaGranularity ||
        NULL == fFunctions.fCreateCommandPool ||
        NULL == fFunctions.fDestroyCommandPool ||
        NULL == fFunctions.fResetCommandPool ||
        NULL == fFunctions.fAllocateCommandBuffers ||
        NULL == fFunctions.fFreeCommandBuffers ||
        NULL == fFunctions.fBeginCommandBuffer ||
        NULL == fFunctions.fEndCommandBuffer ||
        NULL == fFunctions.fResetCommandBuffer ||
        NULL == fFunctions.fCmdBindPipeline ||
        NULL == fFunctions.fCmdSetViewport ||
        NULL == fFunctions.fCmdSetScissor ||
        NULL == fFunctions.fCmdSetLineWidth ||
        NULL == fFunctions.fCmdSetDepthBias ||
        NULL == fFunctions.fCmdSetBlendConstants ||
        NULL == fFunctions.fCmdSetDepthBounds ||
        NULL == fFunctions.fCmdSetStencilCompareMask ||
        NULL == fFunctions.fCmdSetStencilWriteMask ||
        NULL == fFunctions.fCmdSetStencilReference ||
        NULL == fFunctions.fCmdBindDescriptorSets ||
        NULL == fFunctions.fCmdBindIndexBuffer ||
        NULL == fFunctions.fCmdBindVertexBuffers ||
        NULL == fFunctions.fCmdDraw ||
        NULL == fFunctions.fCmdDrawIndexed ||
        NULL == fFunctions.fCmdDrawIndirect ||
        NULL == fFunctions.fCmdDrawIndexedIndirect ||
        NULL == fFunctions.fCmdDispatch ||
        NULL == fFunctions.fCmdDispatchIndirect ||
        NULL == fFunctions.fCmdCopyBuffer ||
        NULL == fFunctions.fCmdCopyImage ||
        NULL == fFunctions.fCmdBlitImage ||
        NULL == fFunctions.fCmdCopyBufferToImage ||
        NULL == fFunctions.fCmdCopyImageToBuffer ||
        NULL == fFunctions.fCmdUpdateBuffer ||
        NULL == fFunctions.fCmdFillBuffer ||
        NULL == fFunctions.fCmdClearColorImage ||
        NULL == fFunctions.fCmdClearDepthStencilImage ||
        NULL == fFunctions.fCmdClearAttachments ||
        NULL == fFunctions.fCmdResolveImage ||
        NULL == fFunctions.fCmdSetEvent ||
        NULL == fFunctions.fCmdResetEvent ||
        NULL == fFunctions.fCmdWaitEvents ||
        NULL == fFunctions.fCmdPipelineBarrier ||
        NULL == fFunctions.fCmdBeginQuery ||
        NULL == fFunctions.fCmdEndQuery ||
        NULL == fFunctions.fCmdResetQueryPool ||
        NULL == fFunctions.fCmdWriteTimestamp ||
        NULL == fFunctions.fCmdCopyQueryPoolResults ||
        NULL == fFunctions.fCmdPushConstants ||
        NULL == fFunctions.fCmdBeginRenderPass ||
        NULL == fFunctions.fCmdNextSubpass ||
        NULL == fFunctions.fCmdEndRenderPass ||
        NULL == fFunctions.fCmdExecuteCommands ||
        NULL == fFunctions.fDestroySurfaceKHR ||
        NULL == fFunctions.fGetPhysicalDeviceSurfaceSupportKHR ||
        NULL == fFunctions.fGetPhysicalDeviceSurfaceCapabilitiesKHR ||
        NULL == fFunctions.fGetPhysicalDeviceSurfaceFormatsKHR ||
        NULL == fFunctions.fGetPhysicalDeviceSurfacePresentModesKHR ||
        NULL == fFunctions.fCreateSwapchainKHR ||
        NULL == fFunctions.fDestroySwapchainKHR ||
        NULL == fFunctions.fGetSwapchainImagesKHR ||
        NULL == fFunctions.fAcquireNextImageKHR ||
        NULL == fFunctions.fQueuePresentKHR ||
        //NULL == fFunctions.fGetPhysicalDeviceDisplayPropertiesKHR ||
        //NULL == fFunctions.fGetPhysicalDeviceDisplayPlanePropertiesKHR ||
        //NULL == fFunctions.fGetDisplayPlaneSupportedDisplaysKHR ||
        //NULL == fFunctions.fGetDisplayModePropertiesKHR ||
        //NULL == fFunctions.fCreateDisplayModeKHR ||
        //NULL == fFunctions.fGetDisplayPlaneCapabilitiesKHR ||
        //NULL == fFunctions.fCreateDisplayPlaneSurfaceKHR ||
        //NULL == fFunctions.fCreateSharedSwapchainsKHR ||
        NULL == fFunctions.fCreateDebugReportCallbackEXT ||
        NULL == fFunctions.fDebugReportMessageEXT ||
        NULL == fFunctions.fDestroyDebugReportCallbackEXT) {

        return false;
    }
    return true;
}
