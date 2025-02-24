/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/private/base/SkMutex.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#if defined(SK_USE_VMA)
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorPriv.h"
#endif

namespace skgpu::graphite {

sk_sp<SharedContext> VulkanSharedContext::Make(const VulkanBackendContext& context,
                                               const ContextOptions& options) {
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
    // If no extensions are provided, make sure we don't have a null dereference downstream.
    skgpu::VulkanExtensions noExtensions;
    const skgpu::VulkanExtensions* extensions = &noExtensions;
    if (context.fVkExtensions) {
        extensions = context.fVkExtensions;
    }

    uint32_t physDevVersion = 0;
    sk_sp<const skgpu::VulkanInterface> interface =
            skgpu::MakeInterface(context, extensions, &physDevVersion, nullptr);
    if (!interface) {
        SKGPU_LOG_E("Failed to create VulkanInterface.");
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
                                                          extensions,
                                                          context.fProtectedContext));

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = context.fMemoryAllocator;
#if defined(SK_USE_VMA)
    if (!memoryAllocator) {
        // We were not given a memory allocator at creation
        skgpu::ThreadSafe threadSafe = options.fClientWillExternallySynchronizeAllThreads
                                               ? skgpu::ThreadSafe::kNo
                                               : skgpu::ThreadSafe::kYes;
        memoryAllocator = skgpu::VulkanMemoryAllocators::Make(context,
                                                              threadSafe,
                                                              options.fVulkanVMALargeHeapBlockSize);
    }
#endif
    if (!memoryAllocator) {
        SKGPU_LOG_E("No supplied vulkan memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new VulkanSharedContext(context,
                                                        std::move(interface),
                                                        std::move(memoryAllocator),
                                                        std::move(caps),
                                                        options.fUserDefinedKnownRuntimeEffects));
}

VulkanSharedContext::VulkanSharedContext(
                const VulkanBackendContext& backendContext,
                sk_sp<const skgpu::VulkanInterface> interface,
                sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator,
                std::unique_ptr<const VulkanCaps> caps,
                SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps), BackendApi::kVulkan, userDefinedKnownRuntimeEffects)
        , fInterface(std::move(interface))
        , fMemoryAllocator(std::move(memoryAllocator))
        , fPhysDevice(backendContext.fPhysicalDevice)
        , fDevice(backendContext.fDevice)
        , fQueueIndex(backendContext.fGraphicsQueueIndex)
        , fDeviceLostContext(backendContext.fDeviceLostContext)
        , fDeviceLostProc(backendContext.fDeviceLostProc) {}

VulkanSharedContext::~VulkanSharedContext() {
    // need to clear out resources before the allocator is removed
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> VulkanSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(
            new VulkanResourceProvider(this,
                                       singleOwner,
                                       recorderID,
                                       resourceBudget));
}

bool VulkanSharedContext::checkVkResult(VkResult result) const {
    switch (result) {
    case VK_SUCCESS:
        return true;
    case VK_ERROR_DEVICE_LOST:
        {
            SkAutoMutexExclusive lock(fDeviceIsLostMutex);
            if (fDeviceIsLost) {
                return false;
            }
            fDeviceIsLost = true;
            // Fall through to InvokeDeviceLostCallback (on first VK_ERROR_DEVICE_LOST) only afer
            // releasing fDeviceIsLostMutex, otherwise clients might cause deadlock by checking
            // isDeviceLost() from the callback.
        }
        skgpu::InvokeDeviceLostCallback(interface(),
                                        device(),
                                        fDeviceLostContext,
                                        fDeviceLostProc,
                                        vulkanCaps().supportsDeviceFaultInfo());
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
