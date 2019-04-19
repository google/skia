/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkAMDMemoryAllocator.h"

#include "GrVkInterface.h"
#include "GrVkMemory.h"
#include "GrVkUtil.h"

GrVkAMDMemoryAllocator::GrVkAMDMemoryAllocator(VkPhysicalDevice physicalDevice,
                                               VkDevice device,
                                               sk_sp<const GrVkInterface> interface)
        : fAllocator(VK_NULL_HANDLE)
        , fInterface(std::move(interface))
        , fDevice(device) {
#define GR_COPY_FUNCTION(NAME) functions.vk##NAME = fInterface->fFunctions.f##NAME

    VmaVulkanFunctions functions;
    GR_COPY_FUNCTION(GetPhysicalDeviceProperties);
    GR_COPY_FUNCTION(GetPhysicalDeviceMemoryProperties);
    GR_COPY_FUNCTION(AllocateMemory);
    GR_COPY_FUNCTION(FreeMemory);
    GR_COPY_FUNCTION(MapMemory);
    GR_COPY_FUNCTION(UnmapMemory);
    GR_COPY_FUNCTION(BindBufferMemory);
    GR_COPY_FUNCTION(BindImageMemory);
    GR_COPY_FUNCTION(GetBufferMemoryRequirements);
    GR_COPY_FUNCTION(GetImageMemoryRequirements);
    GR_COPY_FUNCTION(CreateBuffer);
    GR_COPY_FUNCTION(DestroyBuffer);
    GR_COPY_FUNCTION(CreateImage);
    GR_COPY_FUNCTION(DestroyImage);

    // Skia current doesn't support VK_KHR_dedicated_allocation
    functions.vkGetBufferMemoryRequirements2KHR = nullptr;
    functions.vkGetImageMemoryRequirements2KHR = nullptr;

    VmaAllocatorCreateInfo info;
    info.flags = 0;
    info.physicalDevice = physicalDevice;
    info.device = device;
    // Manually testing runs of dm using 64 here instead of the default 256 shows less memory usage
    // on average. Also dm seems to run faster using 64 so it doesn't seem to be trading off speed
    // for memory.
    info.preferredLargeHeapBlockSize = 4*1024*1024;
    info.pAllocationCallbacks = nullptr;
    info.pDeviceMemoryCallbacks = nullptr;
    info.frameInUseCount = 0;
    info.pHeapSizeLimit = nullptr;
    info.pVulkanFunctions = &functions;

    vmaCreateAllocator(&info, &fAllocator);
}

GrVkAMDMemoryAllocator::~GrVkAMDMemoryAllocator() {
    vmaDestroyAllocator(fAllocator);
    fAllocator = VK_NULL_HANDLE;
}

bool GrVkAMDMemoryAllocator::allocateMemoryForImage(VkImage image, AllocationPropertyFlags flags,
                                                    GrVkBackendMemory* backendMemory) {
    VmaAllocationCreateInfo info;
    info.flags = 0;
    info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    info.preferredFlags = 0;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    if (AllocationPropertyFlags::kDedicatedAllocation & flags) {
        info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    if (AllocationPropertyFlags::kLazyAllocation & flags) {
        info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    VmaAllocation allocation;
    VkResult result = vmaAllocateMemoryForImage(fAllocator, image, &info, &allocation, nullptr);
    if (VK_SUCCESS != result) {
        return false;
    }
    *backendMemory = (GrVkBackendMemory)allocation;
    return true;
}

bool GrVkAMDMemoryAllocator::allocateMemoryForBuffer(VkBuffer buffer, BufferUsage usage,
                                                     AllocationPropertyFlags flags,
                                                     GrVkBackendMemory* backendMemory) {
    VmaAllocationCreateInfo info;
    info.flags = 0;
    info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    switch (usage) {
        case BufferUsage::kGpuOnly:
            info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            info.preferredFlags = 0;
            break;
        case BufferUsage::kCpuOnly:
            info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            info.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            break;
        case BufferUsage::kCpuWritesGpuReads:
            // First attempt to try memory is also cached
            info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case BufferUsage::kGpuWritesCpuReads:
            info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                  VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            break;
    }

    if (AllocationPropertyFlags::kDedicatedAllocation & flags) {
        info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    if ((AllocationPropertyFlags::kLazyAllocation & flags) && BufferUsage::kGpuOnly == usage) {
        info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    if (AllocationPropertyFlags::kPersistentlyMapped & flags) {
        SkASSERT(BufferUsage::kGpuOnly != usage);
        info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocation allocation;
    VkResult result = vmaAllocateMemoryForBuffer(fAllocator, buffer, &info, &allocation, nullptr);
    if (VK_SUCCESS != result) {
        if (usage == BufferUsage::kCpuWritesGpuReads) {
            // We try again but this time drop the requirement for cached
            info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            result = vmaAllocateMemoryForBuffer(fAllocator, buffer, &info, &allocation, nullptr);
        }
    }
    if (VK_SUCCESS != result) {
        return false;
    }

    *backendMemory = (GrVkBackendMemory)allocation;
    return true;
}

void GrVkAMDMemoryAllocator::freeMemory(const GrVkBackendMemory& memoryHandle) {
    const VmaAllocation allocation = (const VmaAllocation)memoryHandle;
    vmaFreeMemory(fAllocator, allocation);
}

void GrVkAMDMemoryAllocator::getAllocInfo(const GrVkBackendMemory& memoryHandle,
                                          GrVkAlloc* alloc) const {
    const VmaAllocation allocation = (const VmaAllocation)memoryHandle;
    VmaAllocationInfo vmaInfo;
    vmaGetAllocationInfo(fAllocator, allocation, &vmaInfo);

    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(fAllocator, vmaInfo.memoryType, &memFlags);

    uint32_t flags = 0;
    if (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & memFlags) {
        flags |= GrVkAlloc::kMappable_Flag;
    }
    if (!SkToBool(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memFlags)) {
        flags |= GrVkAlloc::kNoncoherent_Flag;
    }

    alloc->fMemory        = vmaInfo.deviceMemory;
    alloc->fOffset        = vmaInfo.offset;
    alloc->fSize          = vmaInfo.size;
    alloc->fFlags         = flags;
    alloc->fBackendMemory = memoryHandle;

    // TODO: Remove this hack once the AMD allocator is able to handle the alignment of noncoherent
    // memory itself.
    if (!SkToBool(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memFlags)) {
        // This is a hack to say that the allocation size is actually larger than it is. This is to
        // make sure when we are flushing and invalidating noncoherent memory we have a size that is
        // aligned to the nonCoherentAtomSize. This is safe for three reasons. First the total size
        // of the VkDeviceMemory we allocate will always be a multple of the max possible alignment
        // (currently 256). Second all sub allocations are alignmed with an offset of 256. And
        // finally the allocator we are using always maps the entire VkDeviceMemory so the range
        // we'll be flushing/invalidating will be mapped. So our new fake allocation size will
        // always fit into the VkDeviceMemory, will never push it into another suballocation, and
        // will always be mapped when map is called.
        const VkPhysicalDeviceProperties* devProps;
        vmaGetPhysicalDeviceProperties(fAllocator, &devProps);
        VkDeviceSize alignment = devProps->limits.nonCoherentAtomSize;

        alloc->fSize = (alloc->fSize + alignment - 1) & ~(alignment -1);
    }
}

void* GrVkAMDMemoryAllocator::mapMemory(const GrVkBackendMemory& memoryHandle) {
    const VmaAllocation allocation = (const VmaAllocation)memoryHandle;
    void* mapPtr;
    vmaMapMemory(fAllocator, allocation, &mapPtr);
    return mapPtr;
}

void GrVkAMDMemoryAllocator::unmapMemory(const GrVkBackendMemory& memoryHandle) {
    const VmaAllocation allocation = (const VmaAllocation)memoryHandle;
    vmaUnmapMemory(fAllocator, allocation);
}

void GrVkAMDMemoryAllocator::flushMappedMemory(const GrVkBackendMemory& memoryHandle,
                                               VkDeviceSize offset, VkDeviceSize size) {
    GrVkAlloc info;
    this->getAllocInfo(memoryHandle, &info);

    if (GrVkAlloc::kNoncoherent_Flag & info.fFlags) {
        // We need to store the nonCoherentAtomSize for non-coherent flush/invalidate alignment.
        const VkPhysicalDeviceProperties* physDevProps;
        vmaGetPhysicalDeviceProperties(fAllocator, &physDevProps);
        VkDeviceSize alignment = physDevProps->limits.nonCoherentAtomSize;

        VkMappedMemoryRange mappedMemoryRange;
        GrVkMemory::GetNonCoherentMappedMemoryRange(info, offset, size, alignment,
                                                    &mappedMemoryRange);
        GR_VK_CALL(fInterface, FlushMappedMemoryRanges(fDevice, 1, &mappedMemoryRange));
    }
}

void GrVkAMDMemoryAllocator::invalidateMappedMemory(const GrVkBackendMemory& memoryHandle,
                                                    VkDeviceSize offset, VkDeviceSize size) {
    GrVkAlloc info;
    this->getAllocInfo(memoryHandle, &info);

    if (GrVkAlloc::kNoncoherent_Flag & info.fFlags) {
        // We need to store the nonCoherentAtomSize for non-coherent flush/invalidate alignment.
        const VkPhysicalDeviceProperties* physDevProps;
        vmaGetPhysicalDeviceProperties(fAllocator, &physDevProps);
        VkDeviceSize alignment = physDevProps->limits.nonCoherentAtomSize;

        VkMappedMemoryRange mappedMemoryRange;
        GrVkMemory::GetNonCoherentMappedMemoryRange(info, offset, size, alignment,
                                                    &mappedMemoryRange);
        GR_VK_CALL(fInterface, InvalidateMappedMemoryRanges(fDevice, 1, &mappedMemoryRange));
    }
}

uint64_t GrVkAMDMemoryAllocator::totalUsedMemory() const {
    VmaStats stats;
    vmaCalculateStats(fAllocator, &stats);
    return stats.total.usedBytes;
}

uint64_t GrVkAMDMemoryAllocator::totalAllocatedMemory() const {
    VmaStats stats;
    vmaCalculateStats(fAllocator, &stats);
    return stats.total.usedBytes + stats.total.unusedBytes;
}

