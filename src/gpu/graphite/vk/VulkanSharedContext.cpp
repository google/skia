/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/PersistentPipelineStorage.h"
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
                                                              threadSafe);
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
                                                        options.fExecutor,
                                                        options.fPersistentPipelineStorage,
                                                        options.fUserDefinedKnownRuntimeEffects));
}

VulkanSharedContext::VulkanSharedContext(
                const VulkanBackendContext& backendContext,
                sk_sp<const skgpu::VulkanInterface> interface,
                sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator,
                std::unique_ptr<const VulkanCaps> caps,
                SkExecutor* executor,
                PersistentPipelineStorage* persistentPipelineStorage,
                SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps),
                        BackendApi::kVulkan,
                        executor,
                        userDefinedKnownRuntimeEffects)
        , fInterface(std::move(interface))
        , fMemoryAllocator(std::move(memoryAllocator))
        , fPhysDevice(backendContext.fPhysicalDevice)
        , fDevice(backendContext.fDevice)
        , fQueueIndex(backendContext.fGraphicsQueueIndex)
        , fDeviceLostContext(backendContext.fDeviceLostContext)
        , fDeviceLostProc(backendContext.fDeviceLostProc) {
    fPipelineCache = this->createPipelineCache(backendContext.fPhysicalDevice,
                                               persistentPipelineStorage);
    fThreadSafeResourceProvider = std::make_unique<VulkanThreadSafeResourceProvider>(
        this->makeResourceProvider(&fSingleOwner,
                                   SK_InvalidGenID,
                                   kThreadedSafeResourceBudget));
}

VulkanSharedContext::~VulkanSharedContext() {
    if (fPipelineCache != VK_NULL_HANDLE) {
        VULKAN_CALL(this->interface(),
                    DestroyPipelineCache(this->device(),
                                         fPipelineCache,
                                         nullptr));
        fPipelineCache = VK_NULL_HANDLE;
    }
    fThreadSafeResourceProvider.reset();

    // need to clear out resources before the allocator is removed
    this->globalCache()->deleteResources();
}

VkPipelineCache VulkanSharedContext::createPipelineCache(
        VkPhysicalDevice physDev,
        PersistentPipelineStorage* persistentPipelineStorage) {
    VkPipelineCacheCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;

    sk_sp<SkData> cachedData;  // here to keep the cached data valid past CreatePipelineCache
    if (persistentPipelineStorage) {
        cachedData = persistentPipelineStorage->load();

        // For version one of the header, the total header size is 16 bytes plus
        // VK_UUID_SIZE bytes. See Section 9.6 (Pipeline Cache) in the vulkan spec to see
        // the breakdown of these bytes.
        static const int kV1HeaderSize = 4*sizeof(uint32_t) + VK_UUID_SIZE;
        if (cachedData && cachedData->size() >= kV1HeaderSize) {
            const uint32_t* cacheHeader = (const uint32_t*)cachedData->data();
            if (cacheHeader[1] == VK_PIPELINE_CACHE_HEADER_VERSION_ONE) {
                SkASSERT(cacheHeader[0] == kV1HeaderSize);

                VkPhysicalDeviceProperties2 props;
                props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                props.pNext = nullptr;

                VULKAN_CALL(fInterface.get(), GetPhysicalDeviceProperties2(physDev, &props));

                const VkPhysicalDeviceProperties& physDevProps = props.properties;

                const uint8_t* supportedPipelineCacheUUID = physDevProps.pipelineCacheUUID;
                if (cacheHeader[2] == physDevProps.vendorID &&
                    cacheHeader[3] == physDevProps.deviceID &&
                    memcmp(&cacheHeader[4], supportedPipelineCacheUUID, VK_UUID_SIZE) == 0) {
                        createInfo.initialDataSize = cachedData->size();
                        createInfo.pInitialData = cachedData->data();
                        fLastKnownPersistentPipelineStorageSize = createInfo.initialDataSize;
                }
            }
        }
    }

    VkResult result;
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VULKAN_CALL_RESULT(this,
                       result,
                       CreatePipelineCache(this->device(),
                                           &createInfo,
                                           nullptr,
                                           &pipelineCache));
    if (VK_SUCCESS != result) {
        SKGPU_LOG_W("CreatePipelineCache failed");
        return VK_NULL_HANDLE;
    }

    return pipelineCache;
}

VulkanThreadSafeResourceProvider* VulkanSharedContext::threadSafeResourceProvider() const {
    return static_cast<VulkanThreadSafeResourceProvider*>(fThreadSafeResourceProvider.get());
}

void VulkanSharedContext::syncPipelineData(PersistentPipelineStorage* persistentPipelineStorage,
                                           size_t maxSize) {
    SkASSERT(persistentPipelineStorage);

    if (fPipelineCache == VK_NULL_HANDLE || !fHasNewVkPipelineCacheData) {
        return; // ill-formed SharedContext or no new pipelines
    }

    size_t origDataSize = 0;
    VkResult result;
    VULKAN_CALL_RESULT(
        this,
        result,
        GetPipelineCacheData(this->device(), fPipelineCache, &origDataSize, nullptr));
    if (result != VK_SUCCESS) {
        return;
    }

    if (!this->vulkanCaps().supportsPipelineCreationCacheControl()) {
        // Since we don't have cache control 'fHasNewVkPipelineCacheData' can be wildly
        // inaccurate. Attempt to compensate by comparing the uncapped data sizes.
        if (fLastKnownPersistentPipelineStorageSize == origDataSize) {
            return;
        }
    }

    size_t cappedDataSize = std::min(origDataSize, maxSize);

    std::unique_ptr<uint8_t[]> data(new uint8_t[cappedDataSize]);

    VULKAN_CALL_RESULT_NOCHECK(
        this->interface(),
        result,
        GetPipelineCacheData(this->device(), fPipelineCache, &cappedDataSize, (void*)data.get()));
    this->checkVkResult(result);
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) {
        return;
    }

    fLastKnownPersistentPipelineStorageSize = origDataSize;
    fHasNewVkPipelineCacheData = false;
    persistentPipelineStorage->store(*SkData::MakeWithoutCopy(data.get(), cappedDataSize));
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

sk_sp<GraphicsPipeline> VulkanSharedContext::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    return VulkanGraphicsPipeline::Make(this,
                                        runtimeDict,
                                        pipelineKey,
                                        pipelineDesc,
                                        renderPassDesc,
                                        pipelineCreationFlags,
                                        compilationID);
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
