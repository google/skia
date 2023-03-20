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
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/vk/VulkanAMDMemoryAllocator.h"
#include "src/gpu/vk/VulkanInterface.h"

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

    std::unique_ptr<const VulkanCaps> caps(new VulkanCaps(interface.get(),
                                                          context.fPhysicalDevice,
                                                          physDevVersion,
                                                          context.fVkExtensions,
                                                          options));

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
}

std::unique_ptr<ResourceProvider> VulkanSharedContext::makeResourceProvider(
        SingleOwner* singleOwner) {
    return std::unique_ptr<ResourceProvider>(new VulkanResourceProvider(this, singleOwner));
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

