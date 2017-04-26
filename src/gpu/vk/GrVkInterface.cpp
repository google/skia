/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkUtil.h"

GrVkInterface::GetDeviceProc dev_from_unified(const GrVkInterface::GetProc& get) {
    return [&get](VkDevice device, const char* name) { return get(name, VK_NULL_HANDLE, device); };
}

GrVkInterface::GetInstanceProc instance_from_unified(const GrVkInterface::GetProc& get) {
    return [&get](VkInstance instance, const char* name) {
        return get(name, instance, VK_NULL_HANDLE);
    };
}

GrVkInterface::GrVkInterface(GetProc getProc,
                             VkInstance instance,
                             VkDevice device,
                             uint32_t extensionFlags)
        : GrVkInterface(instance_from_unified(getProc),
                        dev_from_unified(getProc),
                        instance,
                        device,
                        extensionFlags) {}

#define ACQUIRE_INST_PROC(inst, name) \
    fFunctions.f##name = reinterpret_cast<PFN_vk##name>(getInstanceProc(inst, "vk" #name))

#define ACQUIRE_DEV_PROC(dev, name) \
    fFunctions.f##name = reinterpret_cast<PFN_vk##name>((*getDeviceProc)(dev, "vk" #name))

GrVkInterface::GrVkInterface(const GetInstanceProc& getInstanceProc,
                             const GetDeviceProc& origGetDevProc,
                             VkInstance instance,
                             VkDevice device,
                             uint32_t extensionFlags) {
    if (!getInstanceProc || VK_NULL_HANDLE == instance || VK_NULL_HANDLE == device) {
        return;
    }

    GetDeviceProc tempGetDevProc;
    // If the client doesn't provide a device proc getter we fall back onto the instance getter.
    const GetDeviceProc* getDeviceProc = &origGetDevProc;
    if (!origGetDevProc) {
        tempGetDevProc = [&getInstanceProc, instance](VkDevice, const char* name) {
            return getInstanceProc(instance, name);
        };
        getDeviceProc = &tempGetDevProc;
    }

    // Global/Loader Procs.
    ACQUIRE_INST_PROC(VK_NULL_HANDLE, CreateInstance);
    ACQUIRE_INST_PROC(VK_NULL_HANDLE, EnumerateInstanceExtensionProperties);
    ACQUIRE_INST_PROC(VK_NULL_HANDLE, EnumerateInstanceLayerProperties);

    // Instance Procs.
    ACQUIRE_INST_PROC(instance, EnumeratePhysicalDevices);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceFeatures);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceFormatProperties);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceImageFormatProperties);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceProperties);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceQueueFamilyProperties);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceMemoryProperties);
    ACQUIRE_INST_PROC(instance, GetPhysicalDeviceSparseImageFormatProperties);
    ACQUIRE_INST_PROC(instance, DestroyInstance);
    ACQUIRE_INST_PROC(instance, CreateDevice);
    ACQUIRE_INST_PROC(instance, EnumerateDeviceExtensionProperties);
    ACQUIRE_INST_PROC(instance, EnumerateDeviceLayerProperties);

    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        // Also instance Procs.
        ACQUIRE_INST_PROC(instance, CreateDebugReportCallbackEXT);
        ACQUIRE_INST_PROC(instance, DebugReportMessageEXT);
        ACQUIRE_INST_PROC(instance, DestroyDebugReportCallbackEXT);
    }

    // Device Procs.
    ACQUIRE_DEV_PROC(device, DestroyDevice);
    ACQUIRE_DEV_PROC(device, GetDeviceQueue);
    ACQUIRE_DEV_PROC(device, QueueSubmit);
    ACQUIRE_DEV_PROC(device, QueueWaitIdle);
    ACQUIRE_DEV_PROC(device, DeviceWaitIdle);
    ACQUIRE_DEV_PROC(device, AllocateMemory);
    ACQUIRE_DEV_PROC(device, FreeMemory);
    ACQUIRE_DEV_PROC(device, MapMemory);
    ACQUIRE_DEV_PROC(device, UnmapMemory);
    ACQUIRE_DEV_PROC(device, FlushMappedMemoryRanges);
    ACQUIRE_DEV_PROC(device, InvalidateMappedMemoryRanges);
    ACQUIRE_DEV_PROC(device, GetDeviceMemoryCommitment);
    ACQUIRE_DEV_PROC(device, BindBufferMemory);
    ACQUIRE_DEV_PROC(device, BindImageMemory);
    ACQUIRE_DEV_PROC(device, GetBufferMemoryRequirements);
    ACQUIRE_DEV_PROC(device, GetImageMemoryRequirements);
    ACQUIRE_DEV_PROC(device, GetImageSparseMemoryRequirements);
    ACQUIRE_DEV_PROC(device, QueueBindSparse);
    ACQUIRE_DEV_PROC(device, CreateFence);
    ACQUIRE_DEV_PROC(device, DestroyFence);
    ACQUIRE_DEV_PROC(device, ResetFences);
    ACQUIRE_DEV_PROC(device, GetFenceStatus);
    ACQUIRE_DEV_PROC(device, WaitForFences);
    ACQUIRE_DEV_PROC(device, CreateSemaphore);
    ACQUIRE_DEV_PROC(device, DestroySemaphore);
    ACQUIRE_DEV_PROC(device, CreateEvent);
    ACQUIRE_DEV_PROC(device, DestroyEvent);
    ACQUIRE_DEV_PROC(device, GetEventStatus);
    ACQUIRE_DEV_PROC(device, SetEvent);
    ACQUIRE_DEV_PROC(device, ResetEvent);
    ACQUIRE_DEV_PROC(device, CreateQueryPool);
    ACQUIRE_DEV_PROC(device, DestroyQueryPool);
    ACQUIRE_DEV_PROC(device, GetQueryPoolResults);
    ACQUIRE_DEV_PROC(device, CreateBuffer);
    ACQUIRE_DEV_PROC(device, DestroyBuffer);
    ACQUIRE_DEV_PROC(device, CreateBufferView);
    ACQUIRE_DEV_PROC(device, DestroyBufferView);
    ACQUIRE_DEV_PROC(device, CreateImage);
    ACQUIRE_DEV_PROC(device, DestroyImage);
    ACQUIRE_DEV_PROC(device, GetImageSubresourceLayout);
    ACQUIRE_DEV_PROC(device, CreateImageView);
    ACQUIRE_DEV_PROC(device, DestroyImageView);
    ACQUIRE_DEV_PROC(device, CreateShaderModule);
    ACQUIRE_DEV_PROC(device, DestroyShaderModule);
    ACQUIRE_DEV_PROC(device, CreatePipelineCache);
    ACQUIRE_DEV_PROC(device, DestroyPipelineCache);
    ACQUIRE_DEV_PROC(device, GetPipelineCacheData);
    ACQUIRE_DEV_PROC(device, MergePipelineCaches);
    ACQUIRE_DEV_PROC(device, CreateGraphicsPipelines);
    ACQUIRE_DEV_PROC(device, CreateComputePipelines);
    ACQUIRE_DEV_PROC(device, DestroyPipeline);
    ACQUIRE_DEV_PROC(device, CreatePipelineLayout);
    ACQUIRE_DEV_PROC(device, DestroyPipelineLayout);
    ACQUIRE_DEV_PROC(device, CreateSampler);
    ACQUIRE_DEV_PROC(device, DestroySampler);
    ACQUIRE_DEV_PROC(device, CreateDescriptorSetLayout);
    ACQUIRE_DEV_PROC(device, DestroyDescriptorSetLayout);
    ACQUIRE_DEV_PROC(device, CreateDescriptorPool);
    ACQUIRE_DEV_PROC(device, DestroyDescriptorPool);
    ACQUIRE_DEV_PROC(device, ResetDescriptorPool);
    ACQUIRE_DEV_PROC(device, AllocateDescriptorSets);
    ACQUIRE_DEV_PROC(device, FreeDescriptorSets);
    ACQUIRE_DEV_PROC(device, UpdateDescriptorSets);
    ACQUIRE_DEV_PROC(device, CreateFramebuffer);
    ACQUIRE_DEV_PROC(device, DestroyFramebuffer);
    ACQUIRE_DEV_PROC(device, CreateRenderPass);
    ACQUIRE_DEV_PROC(device, DestroyRenderPass);
    ACQUIRE_DEV_PROC(device, GetRenderAreaGranularity);
    ACQUIRE_DEV_PROC(device, CreateCommandPool);
    ACQUIRE_DEV_PROC(device, DestroyCommandPool);
    ACQUIRE_DEV_PROC(device, ResetCommandPool);
    ACQUIRE_DEV_PROC(device, AllocateCommandBuffers);
    ACQUIRE_DEV_PROC(device, FreeCommandBuffers);
    ACQUIRE_DEV_PROC(device, BeginCommandBuffer);
    ACQUIRE_DEV_PROC(device, EndCommandBuffer);
    ACQUIRE_DEV_PROC(device, ResetCommandBuffer);
    ACQUIRE_DEV_PROC(device, CmdBindPipeline);
    ACQUIRE_DEV_PROC(device, CmdSetViewport);
    ACQUIRE_DEV_PROC(device, CmdSetScissor);
    ACQUIRE_DEV_PROC(device, CmdSetLineWidth);
    ACQUIRE_DEV_PROC(device, CmdSetDepthBias);
    ACQUIRE_DEV_PROC(device, CmdSetBlendConstants);
    ACQUIRE_DEV_PROC(device, CmdSetDepthBounds);
    ACQUIRE_DEV_PROC(device, CmdSetStencilCompareMask);
    ACQUIRE_DEV_PROC(device, CmdSetStencilWriteMask);
    ACQUIRE_DEV_PROC(device, CmdSetStencilReference);
    ACQUIRE_DEV_PROC(device, CmdBindDescriptorSets);
    ACQUIRE_DEV_PROC(device, CmdBindIndexBuffer);
    ACQUIRE_DEV_PROC(device, CmdBindVertexBuffers);
    ACQUIRE_DEV_PROC(device, CmdDraw);
    ACQUIRE_DEV_PROC(device, CmdDrawIndexed);
    ACQUIRE_DEV_PROC(device, CmdDrawIndirect);
    ACQUIRE_DEV_PROC(device, CmdDrawIndexedIndirect);
    ACQUIRE_DEV_PROC(device, CmdDispatch);
    ACQUIRE_DEV_PROC(device, CmdDispatchIndirect);
    ACQUIRE_DEV_PROC(device, CmdCopyBuffer);
    ACQUIRE_DEV_PROC(device, CmdCopyImage);
    ACQUIRE_DEV_PROC(device, CmdBlitImage);
    ACQUIRE_DEV_PROC(device, CmdCopyBufferToImage);
    ACQUIRE_DEV_PROC(device, CmdCopyImageToBuffer);
    ACQUIRE_DEV_PROC(device, CmdUpdateBuffer);
    ACQUIRE_DEV_PROC(device, CmdFillBuffer);
    ACQUIRE_DEV_PROC(device, CmdClearColorImage);
    ACQUIRE_DEV_PROC(device, CmdClearDepthStencilImage);
    ACQUIRE_DEV_PROC(device, CmdClearAttachments);
    ACQUIRE_DEV_PROC(device, CmdResolveImage);
    ACQUIRE_DEV_PROC(device, CmdSetEvent);
    ACQUIRE_DEV_PROC(device, CmdResetEvent);
    ACQUIRE_DEV_PROC(device, CmdWaitEvents);
    ACQUIRE_DEV_PROC(device, CmdPipelineBarrier);
    ACQUIRE_DEV_PROC(device, CmdBeginQuery);
    ACQUIRE_DEV_PROC(device, CmdEndQuery);
    ACQUIRE_DEV_PROC(device, CmdResetQueryPool);
    ACQUIRE_DEV_PROC(device, CmdWriteTimestamp);
    ACQUIRE_DEV_PROC(device, CmdCopyQueryPoolResults);
    ACQUIRE_DEV_PROC(device, CmdPushConstants);
    ACQUIRE_DEV_PROC(device, CmdBeginRenderPass);
    ACQUIRE_DEV_PROC(device, CmdNextSubpass);
    ACQUIRE_DEV_PROC(device, CmdEndRenderPass);
    ACQUIRE_DEV_PROC(device, CmdExecuteCommands);
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

