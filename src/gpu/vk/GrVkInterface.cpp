/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkUtil.h"

#define ACQUIRE_PROC(name, instance, device) fFunctions.f##name = \
    reinterpret_cast<PFN_vk##name>(getProc("vk"#name, instance, device));

GrVkInterface::GetProc make_unified_getter(const GrVkInterface::GetInstanceProc& iproc,
                                           const GrVkInterface::GetDeviceProc& dproc) {
    return [&iproc, &dproc](const char* proc_name, VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return dproc(device, proc_name);
        }
        return iproc(instance, proc_name);
    };
}

GrVkInterface::GrVkInterface(const GetInstanceProc& getInstanceProc,
                             const GetDeviceProc& getDeviceProc,
                             VkInstance instance,
                             VkDevice device,
                             uint32_t extensionFlags)
        : GrVkInterface(make_unified_getter(getInstanceProc, getDeviceProc),
                        instance,
                        device,
                        extensionFlags) {}

GrVkInterface::GrVkInterface(GetProc getProc,
                             VkInstance instance,
                             VkDevice device,
                             uint32_t extensionFlags) {
    if (getProc == nullptr) {
        return;
    }
    // Global/Loader Procs.
    ACQUIRE_PROC(CreateInstance, VK_NULL_HANDLE, VK_NULL_HANDLE);
    ACQUIRE_PROC(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);
    ACQUIRE_PROC(EnumerateInstanceLayerProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);

    // Instance Procs.
    ACQUIRE_PROC(EnumeratePhysicalDevices, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceFeatures, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceFormatProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceImageFormatProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceQueueFamilyProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceMemoryProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceSparseImageFormatProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(DestroyInstance, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(CreateDevice, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(DestroyDevice, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(EnumerateDeviceExtensionProperties, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(EnumerateDeviceLayerProperties, instance, VK_NULL_HANDLE);

    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        // Also instance Procs.
        ACQUIRE_PROC(CreateDebugReportCallbackEXT, instance, VK_NULL_HANDLE);
        ACQUIRE_PROC(DebugReportMessageEXT, instance, VK_NULL_HANDLE);
        ACQUIRE_PROC(DestroyDebugReportCallbackEXT, instance, VK_NULL_HANDLE);
    }

    // Device Procs.
    ACQUIRE_PROC(GetDeviceQueue, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(QueueSubmit, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(QueueWaitIdle, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DeviceWaitIdle, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(AllocateMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(FreeMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(MapMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(UnmapMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(FlushMappedMemoryRanges, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(InvalidateMappedMemoryRanges, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetDeviceMemoryCommitment, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(BindBufferMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(BindImageMemory, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetBufferMemoryRequirements, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetImageMemoryRequirements, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetImageSparseMemoryRequirements, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(QueueBindSparse, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateFence, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyFence, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(ResetFences, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetFenceStatus, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(WaitForFences, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateSemaphore, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroySemaphore, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetEventStatus, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(SetEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(ResetEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateQueryPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyQueryPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetQueryPoolResults, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateBufferView, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyBufferView, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetImageSubresourceLayout, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateImageView, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyImageView, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateShaderModule, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyShaderModule, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreatePipelineCache, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyPipelineCache, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetPipelineCacheData, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(MergePipelineCaches, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateGraphicsPipelines, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateComputePipelines, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyPipeline, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreatePipelineLayout, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyPipelineLayout, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateSampler, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroySampler, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateDescriptorSetLayout, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyDescriptorSetLayout, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateDescriptorPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyDescriptorPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(ResetDescriptorPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(AllocateDescriptorSets, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(FreeDescriptorSets, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(UpdateDescriptorSets, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateFramebuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyFramebuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateRenderPass, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyRenderPass, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetRenderAreaGranularity, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CreateCommandPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroyCommandPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(ResetCommandPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(AllocateCommandBuffers, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(FreeCommandBuffers, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(BeginCommandBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(EndCommandBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(ResetCommandBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBindPipeline, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetViewport, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetScissor, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetLineWidth, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetDepthBias, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetBlendConstants, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetDepthBounds, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetStencilCompareMask, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetStencilWriteMask, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetStencilReference, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBindDescriptorSets, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBindIndexBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBindVertexBuffers, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDraw, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDrawIndexed, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDrawIndirect, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDrawIndexedIndirect, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDispatch, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdDispatchIndirect, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdCopyBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdCopyImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBlitImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdCopyBufferToImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdCopyImageToBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdUpdateBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdFillBuffer, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdClearColorImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdClearDepthStencilImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdClearAttachments, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdResolveImage, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdSetEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdResetEvent, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdWaitEvents, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdPipelineBarrier, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBeginQuery, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdEndQuery, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdResetQueryPool, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdWriteTimestamp, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdCopyQueryPoolResults, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdPushConstants, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdBeginRenderPass, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdNextSubpass, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdEndRenderPass, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(CmdExecuteCommands, VK_NULL_HANDLE, device);
}

#ifdef SK_DEBUG
    static int kIsDebug = 1;
#else
    static int kIsDebug = 0;
#endif

#define RETURN_FALSE_INTERFACE                                                                   \
    if (kIsDebug) { SkDebugf("%s:%d GrVkInterface::validate() failed.\n", __FILE__, __LINE__); } \
    return false;

bool GrVkInterface::validate(uint32_t extensionFlags) const {
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
        NULL == fFunctions.fCmdExecuteCommands) {
        RETURN_FALSE_INTERFACE
    }

    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        if (NULL == fFunctions.fCreateDebugReportCallbackEXT ||
            NULL == fFunctions.fDebugReportMessageEXT ||
            NULL == fFunctions.fDestroyDebugReportCallbackEXT) {
            RETURN_FALSE_INTERFACE
        }
    }
    return true;
}

