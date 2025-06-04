/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/vk/VulkanInterface.h"

#include "include/gpu/vk/VulkanExtensions.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"

#include <functional>

namespace skgpu {

#define ACQUIRE_PROC(name, instance, device) \
    fFunctions.f##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_PROC_SUFFIX(name, suffix, instance, device) \
    fFunctions.f##name =                                    \
            reinterpret_cast<PFN_vk##name##suffix>(getProc("vk" #name #suffix, instance, device))

VulkanInterface::VulkanInterface(VulkanGetProc getProc,
                                 VkInstance instance,
                                 VkDevice device,
                                 uint32_t instanceVersion,
                                 uint32_t physicalDeviceVersion,
                                 const VulkanExtensions* extensions) {
    if (getProc == nullptr) {
        return;
    }
    SkASSERT(extensions);

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

    // Functions promoted from VK_KHR_get_physical_device_properties2 to Vulkan 1.1
    ACQUIRE_PROC(GetPhysicalDeviceFeatures2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceProperties2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceFormatProperties2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceImageFormatProperties2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceQueueFamilyProperties2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceMemoryProperties2, instance, VK_NULL_HANDLE);
    ACQUIRE_PROC(GetPhysicalDeviceSparseImageFormatProperties2, instance, VK_NULL_HANDLE);

    // Functions promoted from VK_KHR_get_memory_requirements2 to Vulkan 1.1
    ACQUIRE_PROC(GetImageMemoryRequirements2, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetBufferMemoryRequirements2, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(GetImageSparseMemoryRequirements2, VK_NULL_HANDLE, device);

    // Functions promoted from VK_KHR_bind_memory2 to Vulkan 1.1
    ACQUIRE_PROC(BindBufferMemory2, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(BindImageMemory2, VK_NULL_HANDLE, device);

    // Function promoted from VK_KHR_maintenance1 to Vulkan 1.1
    ACQUIRE_PROC(TrimCommandPool, VK_NULL_HANDLE, device);

    // Function promoted from VK_KHR_maintenance3 to Vulkan 1.1
    ACQUIRE_PROC(GetDescriptorSetLayoutSupport, VK_NULL_HANDLE, device);

    // Function promoted from VK_KHR_external_memory_capabilities to Vulkan 1.1
    ACQUIRE_PROC(GetPhysicalDeviceExternalBufferProperties, instance, VK_NULL_HANDLE);

    // Functions for VK_EXT_extended_dynamic_state or 1.3
    if (physicalDeviceVersion >= VK_API_VERSION_1_3) {
        ACQUIRE_PROC(CmdBindVertexBuffers2, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetCullMode, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetDepthBoundsTestEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetDepthCompareOp, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetDepthTestEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetDepthWriteEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetFrontFace, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetPrimitiveTopology, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetScissorWithCount, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetStencilOp, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetStencilTestEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetViewportWithCount, VK_NULL_HANDLE, device);
    } else if (extensions->hasExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, 1)) {
        ACQUIRE_PROC_SUFFIX(CmdBindVertexBuffers2, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetCullMode, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetDepthBoundsTestEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetDepthCompareOp, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetDepthTestEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetDepthWriteEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetFrontFace, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetPrimitiveTopology, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetScissorWithCount, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetStencilOp, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetStencilTestEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetViewportWithCount, EXT, VK_NULL_HANDLE, device);
    }

    // Functions for VK_EXT_extended_dynamic_state2 or 1.3
    if (physicalDeviceVersion >= VK_API_VERSION_1_3) {
        ACQUIRE_PROC(CmdSetDepthBiasEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetPrimitiveRestartEnable, VK_NULL_HANDLE, device);
        ACQUIRE_PROC(CmdSetRasterizerDiscardEnable, VK_NULL_HANDLE, device);
    } else if (extensions->hasExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, 1)) {
        ACQUIRE_PROC_SUFFIX(CmdSetDepthBiasEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetPrimitiveRestartEnable, EXT, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(CmdSetRasterizerDiscardEnable, EXT, VK_NULL_HANDLE, device);
    }

    // Functions for VK_EXT_vertex_input_dynamic_state
    if (extensions->hasExtension(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, 1)) {
        ACQUIRE_PROC_SUFFIX(CmdSetVertexInput, EXT, VK_NULL_HANDLE, device);
    }

    // Functions promoted from VK_KHR_sampler_ycbcr_conversion to Vulkan 1.1
    ACQUIRE_PROC(CreateSamplerYcbcrConversion, VK_NULL_HANDLE, device);
    ACQUIRE_PROC(DestroySamplerYcbcrConversion, VK_NULL_HANDLE, device);

    // Functions for VK_EXT_device_fault
    if (extensions->hasExtension(VK_EXT_DEVICE_FAULT_EXTENSION_NAME, 1)) {
        ACQUIRE_PROC_SUFFIX(GetDeviceFaultInfo, EXT, VK_NULL_HANDLE, device);
    }

#ifdef SK_BUILD_FOR_ANDROID
    // Functions for VK_ANDROID_external_memory_android_hardware_buffer
    if (extensions->hasExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2)) {
        ACQUIRE_PROC_SUFFIX(GetAndroidHardwareBufferProperties, ANDROID, VK_NULL_HANDLE, device);
        ACQUIRE_PROC_SUFFIX(GetMemoryAndroidHardwareBuffer, ANDROID, VK_NULL_HANDLE, device);
    }
#endif

}

#ifdef SK_DEBUG
    constexpr int kIsDebug = 1;
#else
    constexpr int kIsDebug = 0;
#endif

#define RETURN_FALSE_INTERFACE                                                                     \
    if (kIsDebug) { SkDebugf("%s:%d VulkanInterface::validate() failed.\n", __FILE__, __LINE__); } \
    return false;

bool VulkanInterface::validate(uint32_t instanceVersion,
                               uint32_t physicalDeviceVersion,
                               const VulkanExtensions* extensions) const {
    // functions that are always required
    if (nullptr == fFunctions.fCreateInstance ||
        nullptr == fFunctions.fDestroyInstance ||
        nullptr == fFunctions.fEnumeratePhysicalDevices ||
        nullptr == fFunctions.fGetPhysicalDeviceFeatures ||
        nullptr == fFunctions.fGetPhysicalDeviceFormatProperties ||
        nullptr == fFunctions.fGetPhysicalDeviceImageFormatProperties ||
        nullptr == fFunctions.fGetPhysicalDeviceProperties ||
        nullptr == fFunctions.fGetPhysicalDeviceQueueFamilyProperties ||
        nullptr == fFunctions.fGetPhysicalDeviceMemoryProperties ||
        nullptr == fFunctions.fCreateDevice ||
        nullptr == fFunctions.fDestroyDevice ||
        nullptr == fFunctions.fEnumerateInstanceExtensionProperties ||
        nullptr == fFunctions.fEnumerateDeviceExtensionProperties ||
        nullptr == fFunctions.fEnumerateInstanceLayerProperties ||
        nullptr == fFunctions.fEnumerateDeviceLayerProperties ||
        nullptr == fFunctions.fGetDeviceQueue ||
        nullptr == fFunctions.fQueueSubmit ||
        nullptr == fFunctions.fQueueWaitIdle ||
        nullptr == fFunctions.fDeviceWaitIdle ||
        nullptr == fFunctions.fAllocateMemory ||
        nullptr == fFunctions.fFreeMemory ||
        nullptr == fFunctions.fMapMemory ||
        nullptr == fFunctions.fUnmapMemory ||
        nullptr == fFunctions.fFlushMappedMemoryRanges ||
        nullptr == fFunctions.fInvalidateMappedMemoryRanges ||
        nullptr == fFunctions.fGetDeviceMemoryCommitment ||
        nullptr == fFunctions.fBindBufferMemory ||
        nullptr == fFunctions.fBindImageMemory ||
        nullptr == fFunctions.fGetBufferMemoryRequirements ||
        nullptr == fFunctions.fGetImageMemoryRequirements ||
        nullptr == fFunctions.fGetImageSparseMemoryRequirements ||
        nullptr == fFunctions.fGetPhysicalDeviceSparseImageFormatProperties ||
        nullptr == fFunctions.fQueueBindSparse ||
        nullptr == fFunctions.fCreateFence ||
        nullptr == fFunctions.fDestroyFence ||
        nullptr == fFunctions.fResetFences ||
        nullptr == fFunctions.fGetFenceStatus ||
        nullptr == fFunctions.fWaitForFences ||
        nullptr == fFunctions.fCreateSemaphore ||
        nullptr == fFunctions.fDestroySemaphore ||
        nullptr == fFunctions.fCreateEvent ||
        nullptr == fFunctions.fDestroyEvent ||
        nullptr == fFunctions.fGetEventStatus ||
        nullptr == fFunctions.fSetEvent ||
        nullptr == fFunctions.fResetEvent ||
        nullptr == fFunctions.fCreateQueryPool ||
        nullptr == fFunctions.fDestroyQueryPool ||
        nullptr == fFunctions.fGetQueryPoolResults ||
        nullptr == fFunctions.fCreateBuffer ||
        nullptr == fFunctions.fDestroyBuffer ||
        nullptr == fFunctions.fCreateBufferView ||
        nullptr == fFunctions.fDestroyBufferView ||
        nullptr == fFunctions.fCreateImage ||
        nullptr == fFunctions.fDestroyImage ||
        nullptr == fFunctions.fGetImageSubresourceLayout ||
        nullptr == fFunctions.fCreateImageView ||
        nullptr == fFunctions.fDestroyImageView ||
        nullptr == fFunctions.fCreateShaderModule ||
        nullptr == fFunctions.fDestroyShaderModule ||
        nullptr == fFunctions.fCreatePipelineCache ||
        nullptr == fFunctions.fDestroyPipelineCache ||
        nullptr == fFunctions.fGetPipelineCacheData ||
        nullptr == fFunctions.fMergePipelineCaches ||
        nullptr == fFunctions.fCreateGraphicsPipelines ||
        nullptr == fFunctions.fCreateComputePipelines ||
        nullptr == fFunctions.fDestroyPipeline ||
        nullptr == fFunctions.fCreatePipelineLayout ||
        nullptr == fFunctions.fDestroyPipelineLayout ||
        nullptr == fFunctions.fCreateSampler ||
        nullptr == fFunctions.fDestroySampler ||
        nullptr == fFunctions.fCreateDescriptorSetLayout ||
        nullptr == fFunctions.fDestroyDescriptorSetLayout ||
        nullptr == fFunctions.fCreateDescriptorPool ||
        nullptr == fFunctions.fDestroyDescriptorPool ||
        nullptr == fFunctions.fResetDescriptorPool ||
        nullptr == fFunctions.fAllocateDescriptorSets ||
        nullptr == fFunctions.fFreeDescriptorSets ||
        nullptr == fFunctions.fUpdateDescriptorSets ||
        nullptr == fFunctions.fCreateFramebuffer ||
        nullptr == fFunctions.fDestroyFramebuffer ||
        nullptr == fFunctions.fCreateRenderPass ||
        nullptr == fFunctions.fDestroyRenderPass ||
        nullptr == fFunctions.fGetRenderAreaGranularity ||
        nullptr == fFunctions.fCreateCommandPool ||
        nullptr == fFunctions.fDestroyCommandPool ||
        nullptr == fFunctions.fResetCommandPool ||
        nullptr == fFunctions.fAllocateCommandBuffers ||
        nullptr == fFunctions.fFreeCommandBuffers ||
        nullptr == fFunctions.fBeginCommandBuffer ||
        nullptr == fFunctions.fEndCommandBuffer ||
        nullptr == fFunctions.fResetCommandBuffer ||
        nullptr == fFunctions.fCmdBindPipeline ||
        nullptr == fFunctions.fCmdSetViewport ||
        nullptr == fFunctions.fCmdSetScissor ||
        nullptr == fFunctions.fCmdSetLineWidth ||
        nullptr == fFunctions.fCmdSetDepthBias ||
        nullptr == fFunctions.fCmdSetBlendConstants ||
        nullptr == fFunctions.fCmdSetDepthBounds ||
        nullptr == fFunctions.fCmdSetStencilCompareMask ||
        nullptr == fFunctions.fCmdSetStencilWriteMask ||
        nullptr == fFunctions.fCmdSetStencilReference ||
        nullptr == fFunctions.fCmdBindDescriptorSets ||
        nullptr == fFunctions.fCmdBindIndexBuffer ||
        nullptr == fFunctions.fCmdBindVertexBuffers ||
        nullptr == fFunctions.fCmdDraw ||
        nullptr == fFunctions.fCmdDrawIndexed ||
        nullptr == fFunctions.fCmdDrawIndirect ||
        nullptr == fFunctions.fCmdDrawIndexedIndirect ||
        nullptr == fFunctions.fCmdDispatch ||
        nullptr == fFunctions.fCmdDispatchIndirect ||
        nullptr == fFunctions.fCmdCopyBuffer ||
        nullptr == fFunctions.fCmdCopyImage ||
        nullptr == fFunctions.fCmdBlitImage ||
        nullptr == fFunctions.fCmdCopyBufferToImage ||
        nullptr == fFunctions.fCmdCopyImageToBuffer ||
        nullptr == fFunctions.fCmdUpdateBuffer ||
        nullptr == fFunctions.fCmdFillBuffer ||
        nullptr == fFunctions.fCmdClearColorImage ||
        nullptr == fFunctions.fCmdClearDepthStencilImage ||
        nullptr == fFunctions.fCmdClearAttachments ||
        nullptr == fFunctions.fCmdResolveImage ||
        nullptr == fFunctions.fCmdSetEvent ||
        nullptr == fFunctions.fCmdResetEvent ||
        nullptr == fFunctions.fCmdWaitEvents ||
        nullptr == fFunctions.fCmdPipelineBarrier ||
        nullptr == fFunctions.fCmdBeginQuery ||
        nullptr == fFunctions.fCmdEndQuery ||
        nullptr == fFunctions.fCmdResetQueryPool ||
        nullptr == fFunctions.fCmdWriteTimestamp ||
        nullptr == fFunctions.fCmdCopyQueryPoolResults ||
        nullptr == fFunctions.fCmdPushConstants ||
        nullptr == fFunctions.fCmdBeginRenderPass ||
        nullptr == fFunctions.fCmdNextSubpass ||
        nullptr == fFunctions.fCmdEndRenderPass ||
        nullptr == fFunctions.fCmdExecuteCommands ||
        // Functions promoted from VK_KHR_get_physical_device_properties2 to Vulkan 1.1
        nullptr == fFunctions.fGetPhysicalDeviceFeatures2 ||
        nullptr == fFunctions.fGetPhysicalDeviceProperties2 ||
        nullptr == fFunctions.fGetPhysicalDeviceFormatProperties2 ||
        nullptr == fFunctions.fGetPhysicalDeviceImageFormatProperties2 ||
        nullptr == fFunctions.fGetPhysicalDeviceQueueFamilyProperties2 ||
        nullptr == fFunctions.fGetPhysicalDeviceMemoryProperties2 ||
        nullptr == fFunctions.fGetPhysicalDeviceSparseImageFormatProperties2 ||
        // Functions promoted from VK_KHR_get_memory_requirements2 to Vulkan 1.1
        nullptr == fFunctions.fGetImageMemoryRequirements2 ||
        nullptr == fFunctions.fGetBufferMemoryRequirements2 ||
        nullptr == fFunctions.fGetImageSparseMemoryRequirements2 ||
        // Functions promoted from VK_KHR_bind_memory2 to Vulkan 1.1
        nullptr == fFunctions.fBindBufferMemory2 ||
        nullptr == fFunctions.fBindImageMemory2 ||
        // Function promoted from VK_KHR_maintenance1 to Vulkan 1.1
        nullptr == fFunctions.fTrimCommandPool ||
        // Function promoted from VK_KHR_maintenance3 to Vulkan 1.1
        nullptr == fFunctions.fGetDescriptorSetLayoutSupport ||
        // Function promoted from VK_KHR_external_memory_capabilities to Vulkan 1.1
        nullptr == fFunctions.fGetPhysicalDeviceExternalBufferProperties ||
        // Functions promoted from VK_KHR_sampler_ycbcr_conversion to Vulkan 1.1
        nullptr == fFunctions.fCreateSamplerYcbcrConversion ||
        nullptr == fFunctions.fDestroySamplerYcbcrConversion) {
        RETURN_FALSE_INTERFACE
    }

    // Functions for VK_EXT_extended_dynamic_state or 1.3
    if (physicalDeviceVersion >= VK_API_VERSION_1_3 ||
        extensions->hasExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, 1)) {
        if (nullptr == fFunctions.fCmdBindVertexBuffers2 || nullptr == fFunctions.fCmdSetCullMode ||
            nullptr == fFunctions.fCmdSetDepthBoundsTestEnable ||
            nullptr == fFunctions.fCmdSetDepthCompareOp ||
            nullptr == fFunctions.fCmdSetDepthTestEnable ||
            nullptr == fFunctions.fCmdSetDepthWriteEnable ||
            nullptr == fFunctions.fCmdSetFrontFace ||
            nullptr == fFunctions.fCmdSetPrimitiveTopology ||
            nullptr == fFunctions.fCmdSetScissorWithCount ||
            nullptr == fFunctions.fCmdSetStencilOp ||
            nullptr == fFunctions.fCmdSetStencilTestEnable ||
            nullptr == fFunctions.fCmdSetViewportWithCount) {
            RETURN_FALSE_INTERFACE
        }
    }

    // Functions for VK_EXT_extended_dynamic_state2 or 1.3
    if (physicalDeviceVersion >= VK_API_VERSION_1_3 ||
        extensions->hasExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, 1)) {
        if (nullptr == fFunctions.fCmdSetDepthBiasEnable ||
            nullptr == fFunctions.fCmdSetPrimitiveRestartEnable ||
            nullptr == fFunctions.fCmdSetRasterizerDiscardEnable) {
            RETURN_FALSE_INTERFACE
        }
    }

    // Functions for VK_EXT_vertex_input_dynamic_state
    if (extensions->hasExtension(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, 1)) {
        if (nullptr == fFunctions.fCmdSetVertexInput) {
            RETURN_FALSE_INTERFACE
        }
    }

#ifdef SK_BUILD_FOR_ANDROID
    // Functions for VK_ANDROID_external_memory_android_hardware_buffer
    if (extensions->hasExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2)) {
        if (nullptr == fFunctions.fGetAndroidHardwareBufferProperties ||
            nullptr == fFunctions.fGetMemoryAndroidHardwareBuffer) {
            RETURN_FALSE_INTERFACE
        }
    }
#endif

    return true;
}

} // namespace skgpu

