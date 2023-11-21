/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/vk/VulkanAMDMemoryAllocator.h"
#include "src/gpu/vk/VulkanInterface.h"

namespace skgpu::graphite {

sk_sp<SharedContext> VulkanSharedContext::Make(const VulkanBackendContext& context,
                                               const ContextOptions& options) {
    if (options.fNeverYieldToWebGPU) {
        SKGPU_LOG_W("fNeverYieldToWebGPU is not supported with Vulkan.");
        return nullptr;
    }
    if (context.fInstance == VK_NULL_HANDLE ||
        context.fPhysicalDevice == VK_NULL_HANDLE ||
        context.fDevice == VK_NULL_HANDLE ||
        context.fQueue == VK_NULL_HANDLE) {
        SKGPU_LOG_E("Failed to create VulkanSharedContext because either fInstance,"
                    "fPhysicalDevice, fDevice, or fQueue in the VulkanBackendContext is"
                    "VK_NULL_HANDLE.");
        return nullptr;
    }
    if (!context.fGetProc) {
        SKGPU_LOG_E("Failed to create VulkanSharedContext because there is no valid VulkanGetProc"
                    "on the VulkanBackendContext");
        return nullptr;
    }

    PFN_vkEnumerateInstanceVersion localEnumerateInstanceVersion =
            reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
                    context.fGetProc("vkEnumerateInstanceVersion", VK_NULL_HANDLE, VK_NULL_HANDLE));
    uint32_t instanceVersion = 0;
    if (!localEnumerateInstanceVersion) {
        instanceVersion = VK_MAKE_VERSION(1, 0, 0);
    } else {
        VkResult err = localEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            SKGPU_LOG_E("Failed to enumerate instance version. Err: %d\n", err);
            return nullptr;
        }
    }

    PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                    context.fGetProc("vkGetPhysicalDeviceProperties",
                                      context.fInstance,
                                      VK_NULL_HANDLE));

    if (!localGetPhysicalDeviceProperties) {
        SKGPU_LOG_E("Failed to get function pointer to vkGetPhysicalDeviceProperties.");
        return nullptr;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(context.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    uint32_t apiVersion = context.fMaxAPIVersion ? context.fMaxAPIVersion : instanceVersion;

    instanceVersion = std::min(instanceVersion, apiVersion);
    physDevVersion = std::min(physDevVersion, apiVersion);

    sk_sp<const skgpu::VulkanInterface> interface(
            new skgpu::VulkanInterface(context.fGetProc,
                                       context.fInstance,
                                       context.fDevice,
                                       instanceVersion,
                                       physDevVersion,
                                       context.fVkExtensions));
    if (!interface->validate(instanceVersion, physDevVersion, context.fVkExtensions)) {
        SKGPU_LOG_E("Failed to validate VulkanInterface.");
        return nullptr;
    }

    VkPhysicalDeviceFeatures2 features;
    const VkPhysicalDeviceFeatures2* featuresPtr;
    // If fDeviceFeatures2 is not null, then we ignore fDeviceFeatures. If both are null, we assume
    // no features are enabled.
    if (!context.fDeviceFeatures2 && context.fDeviceFeatures) {
        features.pNext = nullptr;
        features.features = *context.fDeviceFeatures;
        featuresPtr = &features;
    } else {
        featuresPtr = context.fDeviceFeatures2;
    }

    std::unique_ptr<const VulkanCaps> caps(new VulkanCaps(options,
                                                          interface.get(),
                                                          context.fPhysicalDevice,
                                                          physDevVersion,
                                                          featuresPtr,
                                                          context.fVkExtensions,
                                                          context.fProtectedContext));

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = context.fMemoryAllocator;
    if (!memoryAllocator) {
        // TODO: fix this check when we have the caps check
        // We were not given a memory allocator at creation
        bool mustUseCoherentHostVisibleMemory = false; /*caps->mustUseCoherentHostVisibleMemory();*/
        bool threadSafe = !options.fClientWillExternallySynchronizeAllThreads;
        memoryAllocator = skgpu::VulkanAMDMemoryAllocator::Make(context.fInstance,
                                                                context.fPhysicalDevice,
                                                                context.fDevice,
                                                                physDevVersion,
                                                                context.fVkExtensions,
                                                                interface,
                                                                mustUseCoherentHostVisibleMemory,
                                                                threadSafe);
    }
    if (!memoryAllocator) {
        SKGPU_LOG_E("No supplied vulkan memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new VulkanSharedContext(context,
                                                        std::move(interface),
                                                        std::move(memoryAllocator),
                                                        std::move(caps)));
}

VulkanSharedContext::VulkanSharedContext(const VulkanBackendContext& backendContext,
                                         sk_sp<const skgpu::VulkanInterface> interface,
                                         sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator,
                                         std::unique_ptr<const VulkanCaps> caps)
        : skgpu::graphite::SharedContext(std::move(caps), BackendApi::kVulkan)
        , fInterface(std::move(interface))
        , fMemoryAllocator(std::move(memoryAllocator))
        , fDevice(std::move(backendContext.fDevice))
        , fQueueIndex(backendContext.fGraphicsQueueIndex) {}

VulkanSharedContext::~VulkanSharedContext() {
    // need to clear out resources before the allocator is removed
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> VulkanSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    // Establish a uniform buffer that can be updated across multiple render passes and cmd buffers
    size_t alignedIntrinsicConstantSize =
            std::max(VulkanResourceProvider::kIntrinsicConstantSize,
                     this->vulkanCaps().requiredUniformBufferAlignment());
    auto intrinsicConstantBuffer = VulkanBuffer::Make(this,
                                                      alignedIntrinsicConstantSize,
                                                      BufferType::kUniform,
                                                      AccessPattern::kGpuOnly);
    if (!intrinsicConstantBuffer) {
        SKGPU_LOG_E("Failed to create a uniform buffer necessary for VulkanResourceProvider"
                    "creation.");
        return nullptr;
    }
    SkASSERT(static_cast<VulkanBuffer*>(intrinsicConstantBuffer.get())->bufferUsageFlags()
             & VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    return std::unique_ptr<ResourceProvider>(
            new VulkanResourceProvider(this,
                                       singleOwner,
                                       recorderID,
                                       resourceBudget,
                                       std::move(intrinsicConstantBuffer)));
}

bool VulkanSharedContext::checkVkResult(VkResult result) const {
    switch (result) {
    case VK_SUCCESS:
        return true;
    case VK_ERROR_DEVICE_LOST:
        // TODO: determine how we'll track this in a thread-safe manner
        //fDeviceIsLost = true;
        return false;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        // TODO: determine how we'll track this in a thread-safe manner
        //this->setOOMed();
        return false;
    default:
        return false;
    }
}
} // namespace skgpu::graphite
