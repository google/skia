/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkInterface_DEFINED
#define GrVkInterface_DEFINED

#include "SkRefCnt.h"

#include "vk/GrVkDefines.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * The default interface is returned by GrVkCreateInterface. This function's
 * implementation is platform-specific.
 */

struct GrVkInterface;

/**
 * Creates a GrVkInterface.
 */
const GrVkInterface* GrVkCreateInterface(VkInstance instance, VkDevice device,
                                         uint32_t extensionFlags);


/**
 * GrContext uses the following interface to make all calls into Vulkan. When a
 * GrContext is created it is given a GrVkInterface. All functions that should be
 * available based on the Vulkan's version must be non-NULL or GrContext creation
 * will fail. This can be tested with the validate() method.
 */
struct SK_API GrVkInterface : public SkRefCnt {
private:
    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class VkPtr {
    public:
        VkPtr() : fPtr(NULL) {}
        VkPtr operator=(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };

    typedef SkRefCnt INHERITED;

public:
    GrVkInterface();

    // Validates that the GrVkInterface supports its advertised standard. This means the necessary
    // function pointers have been initialized for Vulkan version.
    bool validate() const;

    /**
     * The function pointers are in a struct so that we can have a compiler generated assignment
     * operator.
     */
    struct Functions {
        VkPtr<PFN_vkCreateInstance> fCreateInstance;
        VkPtr<PFN_vkDestroyInstance> fDestroyInstance;
        VkPtr<PFN_vkEnumeratePhysicalDevices> fEnumeratePhysicalDevices;
        VkPtr<PFN_vkGetPhysicalDeviceFeatures> fGetPhysicalDeviceFeatures;
        VkPtr<PFN_vkGetPhysicalDeviceFormatProperties> fGetPhysicalDeviceFormatProperties;
        VkPtr<PFN_vkGetPhysicalDeviceImageFormatProperties> fGetPhysicalDeviceImageFormatProperties;
        VkPtr<PFN_vkGetPhysicalDeviceProperties> fGetPhysicalDeviceProperties;
        VkPtr<PFN_vkGetPhysicalDeviceQueueFamilyProperties> fGetPhysicalDeviceQueueFamilyProperties;
        VkPtr<PFN_vkGetPhysicalDeviceMemoryProperties> fGetPhysicalDeviceMemoryProperties;
        VkPtr<PFN_vkCreateDevice> fCreateDevice;
        VkPtr<PFN_vkDestroyDevice> fDestroyDevice;
        VkPtr<PFN_vkEnumerateInstanceExtensionProperties> fEnumerateInstanceExtensionProperties;
        VkPtr<PFN_vkEnumerateDeviceExtensionProperties> fEnumerateDeviceExtensionProperties;
        VkPtr<PFN_vkEnumerateInstanceLayerProperties> fEnumerateInstanceLayerProperties;
        VkPtr<PFN_vkEnumerateDeviceLayerProperties> fEnumerateDeviceLayerProperties;
        VkPtr<PFN_vkGetDeviceQueue> fGetDeviceQueue;
        VkPtr<PFN_vkQueueSubmit> fQueueSubmit;
        VkPtr<PFN_vkQueueWaitIdle> fQueueWaitIdle;
        VkPtr<PFN_vkDeviceWaitIdle> fDeviceWaitIdle;
        VkPtr<PFN_vkAllocateMemory> fAllocateMemory;
        VkPtr<PFN_vkFreeMemory> fFreeMemory;
        VkPtr<PFN_vkMapMemory> fMapMemory;
        VkPtr<PFN_vkUnmapMemory> fUnmapMemory;
        VkPtr<PFN_vkFlushMappedMemoryRanges> fFlushMappedMemoryRanges;
        VkPtr<PFN_vkInvalidateMappedMemoryRanges> fInvalidateMappedMemoryRanges;
        VkPtr<PFN_vkGetDeviceMemoryCommitment> fGetDeviceMemoryCommitment;
        VkPtr<PFN_vkBindBufferMemory> fBindBufferMemory;
        VkPtr<PFN_vkBindImageMemory> fBindImageMemory;
        VkPtr<PFN_vkGetBufferMemoryRequirements> fGetBufferMemoryRequirements;
        VkPtr<PFN_vkGetImageMemoryRequirements> fGetImageMemoryRequirements;
        VkPtr<PFN_vkGetImageSparseMemoryRequirements> fGetImageSparseMemoryRequirements;
        VkPtr<PFN_vkGetPhysicalDeviceSparseImageFormatProperties> fGetPhysicalDeviceSparseImageFormatProperties;
        VkPtr<PFN_vkQueueBindSparse> fQueueBindSparse;
        VkPtr<PFN_vkCreateFence> fCreateFence;
        VkPtr<PFN_vkDestroyFence> fDestroyFence;
        VkPtr<PFN_vkResetFences> fResetFences;
        VkPtr<PFN_vkGetFenceStatus> fGetFenceStatus;
        VkPtr<PFN_vkWaitForFences> fWaitForFences;
        VkPtr<PFN_vkCreateSemaphore> fCreateSemaphore;
        VkPtr<PFN_vkDestroySemaphore> fDestroySemaphore;
        VkPtr<PFN_vkCreateEvent> fCreateEvent;
        VkPtr<PFN_vkDestroyEvent> fDestroyEvent;
        VkPtr<PFN_vkGetEventStatus> fGetEventStatus;
        VkPtr<PFN_vkSetEvent> fSetEvent;
        VkPtr<PFN_vkResetEvent> fResetEvent;
        VkPtr<PFN_vkCreateQueryPool> fCreateQueryPool;
        VkPtr<PFN_vkDestroyQueryPool> fDestroyQueryPool;
        VkPtr<PFN_vkGetQueryPoolResults> fGetQueryPoolResults;
        VkPtr<PFN_vkCreateBuffer> fCreateBuffer;
        VkPtr<PFN_vkDestroyBuffer> fDestroyBuffer;
        VkPtr<PFN_vkCreateBufferView> fCreateBufferView;
        VkPtr<PFN_vkDestroyBufferView> fDestroyBufferView;
        VkPtr<PFN_vkCreateImage> fCreateImage;
        VkPtr<PFN_vkDestroyImage> fDestroyImage;
        VkPtr<PFN_vkGetImageSubresourceLayout> fGetImageSubresourceLayout;
        VkPtr<PFN_vkCreateImageView> fCreateImageView;
        VkPtr<PFN_vkDestroyImageView> fDestroyImageView;
        VkPtr<PFN_vkCreateShaderModule> fCreateShaderModule;
        VkPtr<PFN_vkDestroyShaderModule> fDestroyShaderModule;
        VkPtr<PFN_vkCreatePipelineCache> fCreatePipelineCache;
        VkPtr<PFN_vkDestroyPipelineCache> fDestroyPipelineCache;
        VkPtr<PFN_vkGetPipelineCacheData> fGetPipelineCacheData;
        VkPtr<PFN_vkMergePipelineCaches> fMergePipelineCaches;
        VkPtr<PFN_vkCreateGraphicsPipelines> fCreateGraphicsPipelines;
        VkPtr<PFN_vkCreateComputePipelines> fCreateComputePipelines;
        VkPtr<PFN_vkDestroyPipeline> fDestroyPipeline;
        VkPtr<PFN_vkCreatePipelineLayout> fCreatePipelineLayout;
        VkPtr<PFN_vkDestroyPipelineLayout> fDestroyPipelineLayout;
        VkPtr<PFN_vkCreateSampler> fCreateSampler;
        VkPtr<PFN_vkDestroySampler> fDestroySampler;
        VkPtr<PFN_vkCreateDescriptorSetLayout> fCreateDescriptorSetLayout;
        VkPtr<PFN_vkDestroyDescriptorSetLayout> fDestroyDescriptorSetLayout;
        VkPtr<PFN_vkCreateDescriptorPool> fCreateDescriptorPool;
        VkPtr<PFN_vkDestroyDescriptorPool> fDestroyDescriptorPool;
        VkPtr<PFN_vkResetDescriptorPool> fResetDescriptorPool;
        VkPtr<PFN_vkAllocateDescriptorSets> fAllocateDescriptorSets;
        VkPtr<PFN_vkFreeDescriptorSets> fFreeDescriptorSets;
        VkPtr<PFN_vkUpdateDescriptorSets> fUpdateDescriptorSets;
        VkPtr<PFN_vkCreateFramebuffer> fCreateFramebuffer;
        VkPtr<PFN_vkDestroyFramebuffer> fDestroyFramebuffer;
        VkPtr<PFN_vkCreateRenderPass> fCreateRenderPass;
        VkPtr<PFN_vkDestroyRenderPass> fDestroyRenderPass;
        VkPtr<PFN_vkGetRenderAreaGranularity> fGetRenderAreaGranularity;
        VkPtr<PFN_vkCreateCommandPool> fCreateCommandPool;
        VkPtr<PFN_vkDestroyCommandPool> fDestroyCommandPool;
        VkPtr<PFN_vkResetCommandPool> fResetCommandPool;
        VkPtr<PFN_vkAllocateCommandBuffers> fAllocateCommandBuffers;
        VkPtr<PFN_vkFreeCommandBuffers> fFreeCommandBuffers;
        VkPtr<PFN_vkBeginCommandBuffer> fBeginCommandBuffer;
        VkPtr<PFN_vkEndCommandBuffer> fEndCommandBuffer;
        VkPtr<PFN_vkResetCommandBuffer> fResetCommandBuffer;
        VkPtr<PFN_vkCmdBindPipeline> fCmdBindPipeline;
        VkPtr<PFN_vkCmdSetViewport> fCmdSetViewport;
        VkPtr<PFN_vkCmdSetScissor> fCmdSetScissor;
        VkPtr<PFN_vkCmdSetLineWidth> fCmdSetLineWidth;
        VkPtr<PFN_vkCmdSetDepthBias> fCmdSetDepthBias;
        VkPtr<PFN_vkCmdSetBlendConstants> fCmdSetBlendConstants;
        VkPtr<PFN_vkCmdSetDepthBounds> fCmdSetDepthBounds;
        VkPtr<PFN_vkCmdSetStencilCompareMask> fCmdSetStencilCompareMask;
        VkPtr<PFN_vkCmdSetStencilWriteMask> fCmdSetStencilWriteMask;
        VkPtr<PFN_vkCmdSetStencilReference> fCmdSetStencilReference;
        VkPtr<PFN_vkCmdBindDescriptorSets> fCmdBindDescriptorSets;
        VkPtr<PFN_vkCmdBindIndexBuffer> fCmdBindIndexBuffer;
        VkPtr<PFN_vkCmdBindVertexBuffers> fCmdBindVertexBuffers;
        VkPtr<PFN_vkCmdDraw> fCmdDraw;
        VkPtr<PFN_vkCmdDrawIndexed> fCmdDrawIndexed;
        VkPtr<PFN_vkCmdDrawIndirect> fCmdDrawIndirect;
        VkPtr<PFN_vkCmdDrawIndexedIndirect> fCmdDrawIndexedIndirect;
        VkPtr<PFN_vkCmdDispatch> fCmdDispatch;
        VkPtr<PFN_vkCmdDispatchIndirect> fCmdDispatchIndirect;
        VkPtr<PFN_vkCmdCopyBuffer> fCmdCopyBuffer;
        VkPtr<PFN_vkCmdCopyImage> fCmdCopyImage;
        VkPtr<PFN_vkCmdBlitImage> fCmdBlitImage;
        VkPtr<PFN_vkCmdCopyBufferToImage> fCmdCopyBufferToImage;
        VkPtr<PFN_vkCmdCopyImageToBuffer> fCmdCopyImageToBuffer;
        VkPtr<PFN_vkCmdUpdateBuffer> fCmdUpdateBuffer;
        VkPtr<PFN_vkCmdFillBuffer> fCmdFillBuffer;
        VkPtr<PFN_vkCmdClearColorImage> fCmdClearColorImage;
        VkPtr<PFN_vkCmdClearDepthStencilImage> fCmdClearDepthStencilImage;
        VkPtr<PFN_vkCmdClearAttachments> fCmdClearAttachments;
        VkPtr<PFN_vkCmdResolveImage> fCmdResolveImage;
        VkPtr<PFN_vkCmdSetEvent> fCmdSetEvent;
        VkPtr<PFN_vkCmdResetEvent> fCmdResetEvent;
        VkPtr<PFN_vkCmdWaitEvents> fCmdWaitEvents;
        VkPtr<PFN_vkCmdPipelineBarrier> fCmdPipelineBarrier;
        VkPtr<PFN_vkCmdBeginQuery> fCmdBeginQuery;
        VkPtr<PFN_vkCmdEndQuery> fCmdEndQuery;
        VkPtr<PFN_vkCmdResetQueryPool> fCmdResetQueryPool;
        VkPtr<PFN_vkCmdWriteTimestamp> fCmdWriteTimestamp;
        VkPtr<PFN_vkCmdCopyQueryPoolResults> fCmdCopyQueryPoolResults;
        VkPtr<PFN_vkCmdPushConstants> fCmdPushConstants;
        VkPtr<PFN_vkCmdBeginRenderPass> fCmdBeginRenderPass;
        VkPtr<PFN_vkCmdNextSubpass> fCmdNextSubpass;
        VkPtr<PFN_vkCmdEndRenderPass> fCmdEndRenderPass;
        VkPtr<PFN_vkCmdExecuteCommands> fCmdExecuteCommands;

        VkPtr<PFN_vkCreateDebugReportCallbackEXT> fCreateDebugReportCallbackEXT;
        VkPtr<PFN_vkDebugReportMessageEXT> fDebugReportMessageEXT;
        VkPtr<PFN_vkDestroyDebugReportCallbackEXT> fDestroyDebugReportCallbackEXT;
    } fFunctions;

};

#endif
