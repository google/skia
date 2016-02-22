/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkInterface.h"

GrVkInterface::GrVkInterface() {
}

#define GET_PROC(F) functions->f ## F = (PFN_vk ## F) vkGetInstanceProcAddr(instance, "vk" #F)

const GrVkInterface* GrVkCreateInterface(VkInstance instance) {

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
    GET_PROC(GetDeviceQueue);
    GET_PROC(QueueSubmit);
    GET_PROC(QueueWaitIdle);
    GET_PROC(DeviceWaitIdle);
    GET_PROC(AllocateMemory);
    GET_PROC(FreeMemory);
    GET_PROC(MapMemory);
    GET_PROC(UnmapMemory);
    GET_PROC(FlushMappedMemoryRanges);
    GET_PROC(InvalidateMappedMemoryRanges);
    GET_PROC(GetDeviceMemoryCommitment);
    GET_PROC(BindBufferMemory);
    GET_PROC(BindImageMemory);
    GET_PROC(GetBufferMemoryRequirements);
    GET_PROC(GetImageMemoryRequirements);
    GET_PROC(GetImageSparseMemoryRequirements);
    GET_PROC(GetPhysicalDeviceSparseImageFormatProperties);
    GET_PROC(QueueBindSparse);
    GET_PROC(CreateFence);
    GET_PROC(DestroyFence);
    GET_PROC(ResetFences);
    GET_PROC(GetFenceStatus);
    GET_PROC(WaitForFences);
    GET_PROC(CreateSemaphore);
    GET_PROC(DestroySemaphore);
    GET_PROC(CreateEvent);
    GET_PROC(DestroyEvent);
    GET_PROC(GetEventStatus);
    GET_PROC(SetEvent);
    GET_PROC(ResetEvent);
    GET_PROC(CreateQueryPool);
    GET_PROC(DestroyQueryPool);
    GET_PROC(GetQueryPoolResults);
    GET_PROC(CreateBuffer);
    GET_PROC(DestroyBuffer);
    GET_PROC(CreateBufferView);
    GET_PROC(DestroyBufferView);
    GET_PROC(CreateImage);
    GET_PROC(DestroyImage);
    GET_PROC(GetImageSubresourceLayout);
    GET_PROC(CreateImageView);
    GET_PROC(DestroyImageView);
    GET_PROC(CreateShaderModule);
    GET_PROC(DestroyShaderModule);
    GET_PROC(CreatePipelineCache);
    GET_PROC(DestroyPipelineCache);
    GET_PROC(GetPipelineCacheData);
    GET_PROC(MergePipelineCaches);
    GET_PROC(CreateGraphicsPipelines);
    GET_PROC(CreateComputePipelines);
    GET_PROC(DestroyPipeline);
    GET_PROC(CreatePipelineLayout);
    GET_PROC(DestroyPipelineLayout);
    GET_PROC(CreateSampler);
    GET_PROC(DestroySampler);
    GET_PROC(CreateDescriptorSetLayout);
    GET_PROC(DestroyDescriptorSetLayout);
    GET_PROC(CreateDescriptorPool);
    GET_PROC(DestroyDescriptorPool);
    GET_PROC(ResetDescriptorPool);
    GET_PROC(AllocateDescriptorSets);
    GET_PROC(FreeDescriptorSets);
    GET_PROC(UpdateDescriptorSets);
    GET_PROC(CreateFramebuffer);
    GET_PROC(DestroyFramebuffer);
    GET_PROC(CreateRenderPass);
    GET_PROC(DestroyRenderPass);
    GET_PROC(GetRenderAreaGranularity);
    GET_PROC(CreateCommandPool);
    GET_PROC(DestroyCommandPool);
    GET_PROC(ResetCommandPool);
    GET_PROC(AllocateCommandBuffers);
    GET_PROC(FreeCommandBuffers);
    GET_PROC(BeginCommandBuffer);
    GET_PROC(EndCommandBuffer);
    GET_PROC(ResetCommandBuffer);
    GET_PROC(CmdBindPipeline);
    GET_PROC(CmdSetViewport);
    GET_PROC(CmdSetScissor);
    GET_PROC(CmdSetLineWidth);
    GET_PROC(CmdSetDepthBias);
    GET_PROC(CmdSetBlendConstants);
    GET_PROC(CmdSetDepthBounds);
    GET_PROC(CmdSetStencilCompareMask);
    GET_PROC(CmdSetStencilWriteMask);
    GET_PROC(CmdSetStencilReference);
    GET_PROC(CmdBindDescriptorSets);
    GET_PROC(CmdBindIndexBuffer);
    GET_PROC(CmdBindVertexBuffers);
    GET_PROC(CmdDraw);
    GET_PROC(CmdDrawIndexed);
    GET_PROC(CmdDrawIndirect);
    GET_PROC(CmdDrawIndexedIndirect);
    GET_PROC(CmdDispatch);
    GET_PROC(CmdDispatchIndirect);
    GET_PROC(CmdCopyBuffer);
    GET_PROC(CmdCopyImage);
    GET_PROC(CmdBlitImage);
    GET_PROC(CmdCopyBufferToImage);
    GET_PROC(CmdCopyImageToBuffer);
    GET_PROC(CmdUpdateBuffer);
    GET_PROC(CmdFillBuffer);
    GET_PROC(CmdClearColorImage);
    GET_PROC(CmdClearDepthStencilImage);
    GET_PROC(CmdClearAttachments);
    GET_PROC(CmdResolveImage);
    GET_PROC(CmdSetEvent);
    GET_PROC(CmdResetEvent);
    GET_PROC(CmdWaitEvents);
    GET_PROC(CmdPipelineBarrier);
    GET_PROC(CmdBeginQuery);
    GET_PROC(CmdEndQuery);
    GET_PROC(CmdResetQueryPool);
    GET_PROC(CmdWriteTimestamp);
    GET_PROC(CmdCopyQueryPoolResults);
    GET_PROC(CmdPushConstants);
    GET_PROC(CmdBeginRenderPass);
    GET_PROC(CmdNextSubpass);
    GET_PROC(CmdEndRenderPass);
    GET_PROC(CmdExecuteCommands);
    GET_PROC(DestroySurfaceKHR);
    GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
    GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
    GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    GET_PROC(CreateSwapchainKHR);
    GET_PROC(DestroySwapchainKHR);
    GET_PROC(GetSwapchainImagesKHR);
    GET_PROC(AcquireNextImageKHR);
    GET_PROC(QueuePresentKHR);
    GET_PROC(GetPhysicalDeviceDisplayPropertiesKHR);
    GET_PROC(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    GET_PROC(GetDisplayPlaneSupportedDisplaysKHR);
    GET_PROC(GetDisplayModePropertiesKHR);
    GET_PROC(CreateDisplayModeKHR);
    GET_PROC(GetDisplayPlaneCapabilitiesKHR);
    GET_PROC(CreateDisplayPlaneSurfaceKHR);
    GET_PROC(CreateSharedSwapchainsKHR);

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
        NULL == fFunctions.fGetPhysicalDeviceDisplayPropertiesKHR ||
        NULL == fFunctions.fGetPhysicalDeviceDisplayPlanePropertiesKHR ||
        NULL == fFunctions.fGetDisplayPlaneSupportedDisplaysKHR ||
        NULL == fFunctions.fGetDisplayModePropertiesKHR ||
        NULL == fFunctions.fCreateDisplayModeKHR ||
        NULL == fFunctions.fGetDisplayPlaneCapabilitiesKHR ||
        NULL == fFunctions.fCreateDisplayPlaneSurfaceKHR ||
        NULL == fFunctions.fCreateSharedSwapchainsKHR) {
        return false;
    }
    return true;
}

