/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkUtil.h"

GrVkInterface::GrVkInterface(GetProc proc, uint32_t extensionFlags) {
    if (proc == nullptr) {
        return;
    }

#define RESOLVE_PROC(name) fFunctions.f##name = reinterpret_cast<PFN_vk##name>(proc("vk" #name));

    RESOLVE_PROC(CreateInstance);
    RESOLVE_PROC(EnumerateInstanceExtensionProperties);
    RESOLVE_PROC(EnumerateInstanceLayerProperties);
    RESOLVE_PROC(DestroyInstance);
    RESOLVE_PROC(EnumeratePhysicalDevices);
    RESOLVE_PROC(GetPhysicalDeviceFeatures);
    RESOLVE_PROC(GetPhysicalDeviceFormatProperties);
    RESOLVE_PROC(GetPhysicalDeviceImageFormatProperties);
    RESOLVE_PROC(GetPhysicalDeviceProperties);
    RESOLVE_PROC(GetPhysicalDeviceQueueFamilyProperties);
    RESOLVE_PROC(GetPhysicalDeviceMemoryProperties);
    RESOLVE_PROC(CreateDevice);
    RESOLVE_PROC(DestroyDevice);
    RESOLVE_PROC(EnumerateDeviceExtensionProperties);
    RESOLVE_PROC(EnumerateDeviceLayerProperties);
    RESOLVE_PROC(GetDeviceQueue);
    RESOLVE_PROC(QueueSubmit);
    RESOLVE_PROC(QueueWaitIdle);
    RESOLVE_PROC(DeviceWaitIdle);
    RESOLVE_PROC(AllocateMemory);
    RESOLVE_PROC(FreeMemory);
    RESOLVE_PROC(MapMemory);
    RESOLVE_PROC(UnmapMemory);
    RESOLVE_PROC(FlushMappedMemoryRanges);
    RESOLVE_PROC(InvalidateMappedMemoryRanges);
    RESOLVE_PROC(GetDeviceMemoryCommitment);
    RESOLVE_PROC(BindBufferMemory);
    RESOLVE_PROC(BindImageMemory);
    RESOLVE_PROC(GetBufferMemoryRequirements);
    RESOLVE_PROC(GetImageMemoryRequirements);
    RESOLVE_PROC(GetImageSparseMemoryRequirements);
    RESOLVE_PROC(GetPhysicalDeviceSparseImageFormatProperties);
    RESOLVE_PROC(QueueBindSparse);
    RESOLVE_PROC(CreateFence);
    RESOLVE_PROC(DestroyFence);
    RESOLVE_PROC(ResetFences);
    RESOLVE_PROC(GetFenceStatus);
    RESOLVE_PROC(WaitForFences);
    RESOLVE_PROC(CreateSemaphore);
    RESOLVE_PROC(DestroySemaphore);
    RESOLVE_PROC(CreateEvent);
    RESOLVE_PROC(DestroyEvent);
    RESOLVE_PROC(GetEventStatus);
    RESOLVE_PROC(SetEvent);
    RESOLVE_PROC(ResetEvent);
    RESOLVE_PROC(CreateQueryPool);
    RESOLVE_PROC(DestroyQueryPool);
    RESOLVE_PROC(GetQueryPoolResults);
    RESOLVE_PROC(CreateBuffer);
    RESOLVE_PROC(DestroyBuffer);
    RESOLVE_PROC(CreateBufferView);
    RESOLVE_PROC(DestroyBufferView);
    RESOLVE_PROC(CreateImage);
    RESOLVE_PROC(DestroyImage);
    RESOLVE_PROC(GetImageSubresourceLayout);
    RESOLVE_PROC(CreateImageView);
    RESOLVE_PROC(DestroyImageView);
    RESOLVE_PROC(CreateShaderModule);
    RESOLVE_PROC(DestroyShaderModule);
    RESOLVE_PROC(CreatePipelineCache);
    RESOLVE_PROC(DestroyPipelineCache);
    RESOLVE_PROC(GetPipelineCacheData);
    RESOLVE_PROC(MergePipelineCaches);
    RESOLVE_PROC(CreateGraphicsPipelines);
    RESOLVE_PROC(CreateComputePipelines);
    RESOLVE_PROC(DestroyPipeline);
    RESOLVE_PROC(CreatePipelineLayout);
    RESOLVE_PROC(DestroyPipelineLayout);
    RESOLVE_PROC(CreateSampler);
    RESOLVE_PROC(DestroySampler);
    RESOLVE_PROC(CreateDescriptorSetLayout);
    RESOLVE_PROC(DestroyDescriptorSetLayout);
    RESOLVE_PROC(CreateDescriptorPool);
    RESOLVE_PROC(DestroyDescriptorPool);
    RESOLVE_PROC(ResetDescriptorPool);
    RESOLVE_PROC(AllocateDescriptorSets);
    RESOLVE_PROC(FreeDescriptorSets);
    RESOLVE_PROC(UpdateDescriptorSets);
    RESOLVE_PROC(CreateFramebuffer);
    RESOLVE_PROC(DestroyFramebuffer);
    RESOLVE_PROC(CreateRenderPass);
    RESOLVE_PROC(DestroyRenderPass);
    RESOLVE_PROC(GetRenderAreaGranularity);
    RESOLVE_PROC(CreateCommandPool);
    RESOLVE_PROC(DestroyCommandPool);
    RESOLVE_PROC(ResetCommandPool);
    RESOLVE_PROC(AllocateCommandBuffers);
    RESOLVE_PROC(FreeCommandBuffers);
    RESOLVE_PROC(BeginCommandBuffer);
    RESOLVE_PROC(EndCommandBuffer);
    RESOLVE_PROC(ResetCommandBuffer);
    RESOLVE_PROC(CmdBindPipeline);
    RESOLVE_PROC(CmdSetViewport);
    RESOLVE_PROC(CmdSetScissor);
    RESOLVE_PROC(CmdSetLineWidth);
    RESOLVE_PROC(CmdSetDepthBias);
    RESOLVE_PROC(CmdSetBlendConstants);
    RESOLVE_PROC(CmdSetDepthBounds);
    RESOLVE_PROC(CmdSetStencilCompareMask);
    RESOLVE_PROC(CmdSetStencilWriteMask);
    RESOLVE_PROC(CmdSetStencilReference);
    RESOLVE_PROC(CmdBindDescriptorSets);
    RESOLVE_PROC(CmdBindIndexBuffer);
    RESOLVE_PROC(CmdBindVertexBuffers);
    RESOLVE_PROC(CmdDraw);
    RESOLVE_PROC(CmdDrawIndexed);
    RESOLVE_PROC(CmdDrawIndirect);
    RESOLVE_PROC(CmdDrawIndexedIndirect);
    RESOLVE_PROC(CmdDispatch);
    RESOLVE_PROC(CmdDispatchIndirect);
    RESOLVE_PROC(CmdCopyBuffer);
    RESOLVE_PROC(CmdCopyImage);
    RESOLVE_PROC(CmdBlitImage);
    RESOLVE_PROC(CmdCopyBufferToImage);
    RESOLVE_PROC(CmdCopyImageToBuffer);
    RESOLVE_PROC(CmdUpdateBuffer);
    RESOLVE_PROC(CmdFillBuffer);
    RESOLVE_PROC(CmdClearColorImage);
    RESOLVE_PROC(CmdClearDepthStencilImage);
    RESOLVE_PROC(CmdClearAttachments);
    RESOLVE_PROC(CmdResolveImage);
    RESOLVE_PROC(CmdSetEvent);
    RESOLVE_PROC(CmdResetEvent);
    RESOLVE_PROC(CmdWaitEvents);
    RESOLVE_PROC(CmdPipelineBarrier);
    RESOLVE_PROC(CmdBeginQuery);
    RESOLVE_PROC(CmdEndQuery);
    RESOLVE_PROC(CmdResetQueryPool);
    RESOLVE_PROC(CmdWriteTimestamp);
    RESOLVE_PROC(CmdCopyQueryPoolResults);
    RESOLVE_PROC(CmdPushConstants);
    RESOLVE_PROC(CmdBeginRenderPass);
    RESOLVE_PROC(CmdNextSubpass);
    RESOLVE_PROC(CmdEndRenderPass);
    RESOLVE_PROC(CmdExecuteCommands);

    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        RESOLVE_PROC(CreateDebugReportCallbackEXT);
        RESOLVE_PROC(DebugReportMessageEXT);
        RESOLVE_PROC(DestroyDebugReportCallbackEXT);
    }

#undef RESOLVE_PROC
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

