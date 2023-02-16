/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanBuffer.h"

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/vk/VulkanMemory.h"

namespace skgpu::graphite {

sk_sp<Buffer> VulkanBuffer::Make(const VulkanSharedContext* sharedContext,
                                 size_t size,
                                 BufferType type,
                                 PrioritizeGpuReads prioritizeGpuReads) {
    if (size <= 0) {
        return nullptr;
    }
    VkBuffer buffer;
    skgpu::VulkanAlloc alloc;

    // The only time we don't require mappable buffers is when we're on a device where gpu only
    // memory has faster reads on the gpu than memory that is also mappable on the cpu. Protected
    // memory always uses mappable buffers.
    bool requiresMappable = sharedContext->isProtected() == Protected::kYes ||
                            prioritizeGpuReads == PrioritizeGpuReads::kNo ||
                            !sharedContext->vulkanCaps().gpuOnlyBuffersMorePerformant();

    using BufferUsage = skgpu::VulkanMemoryAllocator::BufferUsage;

    // The default usage captures use cases besides transfer buffers. GPU-only buffers are preferred
    // unless mappability is required.
    BufferUsage allocUsage =
            requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;

    // Create the buffer object
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = 0;
    bufInfo.size = size;

    // To support SkMesh buffer updates we make Vertex and Index buffers capable of being transfer
    // dsts.
    switch (type) {
        case BufferType::kVertex:
            bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferType::kIndex:
            bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferType::kStorage:
            bufInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferType::kIndirect:
            bufInfo.usage =
                    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferType::kVertexStorage:
            bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferType::kIndexStorage:
            bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferType::kUniform:
            bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            allocUsage = BufferUsage::kCpuWritesGpuReads;
            break;
        case BufferType::kXferCpuToGpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocUsage = BufferUsage::kTransfersFromCpuToGpu;
            break;
        case BufferType::kXferGpuToCpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocUsage = BufferUsage::kTransfersFromGpuToCpu;
            break;
    }

    // We may not always get a mappable buffer for non-dynamic access buffers. Thus we set the
    // transfer dst usage bit in case we need to do a copy to write data. It doesn't really hurt
    // to set this extra usage flag, but we could narrow the scope of buffers we set it on more than
    // just not dynamic.
    if (!requiresMappable) {
        bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;

    VkResult result;
    VULKAN_CALL_RESULT(sharedContext->interface(), result, CreateBuffer(sharedContext->device(),
                                                           &bufInfo,
                                                           nullptr, /*const VkAllocationCallbacks*/
                                                           &buffer));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    auto allocator = sharedContext->memoryAllocator();
    bool shouldPersistentlyMapCpuToGpu =
        sharedContext->vulkanCaps().shouldPersistentlyMapCpuToGpuBuffers();
    //AllocBufferMemory
    auto checkResult = [](VkResult result) {
        return result == VK_SUCCESS;
    };
    if (!skgpu::VulkanMemory::AllocBufferMemory(allocator,
                                                buffer,
                                                allocUsage,
                                                shouldPersistentlyMapCpuToGpu,
                                                checkResult,
                                                &alloc)) {
        VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                buffer,
                /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    // Bind buffer
    VULKAN_CALL_RESULT(sharedContext->interface(), result, BindBufferMemory(sharedContext->device(),
                                                                            buffer,
                                                                            alloc.fMemory,
                                                                            alloc.fOffset));
    if (result != VK_SUCCESS) {
        skgpu::VulkanMemory::FreeBufferMemory(allocator, alloc);
        VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                buffer,
                /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    // TODO: If this is a uniform buffer, we must set up a descriptor set.
    // const GrVkDescriptorSet* uniformDescSet = nullptr;
    // if (bufferType == kUniform::kUniform) {
    //     uniformDescSet = make_uniform_desc_set(gpu, buffer, size);
    //     if (!uniformDescSet) {
    //         VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
    //         skgpu::VulkanMemory::FreeBufferMemory(allocator, alloc);
    //         return nullptr;
    //     }
    // }

    return sk_sp<Buffer>(new VulkanBuffer(sharedContext,
                                          size,
                                          type,
                                          prioritizeGpuReads,
                                          std::move(buffer),
                                          alloc));
}

VulkanBuffer::VulkanBuffer(const VulkanSharedContext* sharedContext,
                           size_t size,
                           BufferType type,
                           PrioritizeGpuReads prioritizeGpuReads,
                           VkBuffer buffer,
                           const skgpu::VulkanAlloc& alloc)
        : Buffer(sharedContext, size)
        , fBuffer(std::move(buffer))
        , fAlloc(alloc)
        // We assume a buffer is used for CPU reads only in the case of GPU->CPU transfer buffers.
        , fBufferUsedForCPURead(type == BufferType::kXferGpuToCpu) {}

void VulkanBuffer::freeGpuData() {
    if (fMapPtr) {
        this->internalUnmap(0, this->size());
        fMapPtr = nullptr;
    }

    // TODO: If this is a uniform buffer, we must clean up the descriptor set.
    //if (fUniformDescriptorSet) {
    //    fUniformDescriptorSet->recycle();
    //    fUniformDescriptorSet = nullptr;
    //}

    const VulkanSharedContext* sharedContext =
            static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fBuffer);
    SkASSERT(fAlloc.fMemory && fAlloc.fBackendMemory);
    VULKAN_CALL(sharedContext->interface(),
                DestroyBuffer(sharedContext->device(), fBuffer, nullptr));
    fBuffer = VK_NULL_HANDLE;

    skgpu::VulkanMemory::FreeBufferMemory(sharedContext->memoryAllocator(), fAlloc);
    fAlloc.fMemory = VK_NULL_HANDLE;
    fAlloc.fBackendMemory = 0;
}

void VulkanBuffer::internalMap(size_t readOffset, size_t readSize) {
    SkASSERT(!fMapPtr);
    if (this->isMappable()) {
        // Not every buffer will use command buffer usage refs. Instead, the command buffer just
        // holds normal refs. Systems higher up in Graphite should be making sure not to reuse a
        // buffer that currently has a ref held by something else. However, we do need to make sure
        // there isn't a buffer with just a command buffer usage that is trying to be mapped.
#ifdef SK_DEBUG
        SkASSERT(!this->debugHasCommandBufferRef());
#endif
        SkASSERT(fAlloc.fSize > 0);
        SkASSERT(fAlloc.fSize >= readOffset + readSize);

        const VulkanSharedContext* sharedContext = this->vulkanSharedContext();

        auto allocator = sharedContext->memoryAllocator();
        auto checkResult = [sharedContext](VkResult result) {
            return sharedContext->checkVkResult(result);
        };
        fMapPtr = skgpu::VulkanMemory::MapAlloc(allocator, fAlloc, checkResult);
        if (fMapPtr && readSize != 0) {
            // "Invalidate" here means make device writes visible to the host. That is, it makes
            // sure any GPU writes are finished in the range we might read from.
            skgpu::VulkanMemory::InvalidateMappedAlloc(allocator,
                                                       fAlloc,
                                                       readOffset,
                                                       readSize,
                                                       nullptr);
        }
    }
}

void VulkanBuffer::internalUnmap(size_t flushOffset, size_t flushSize) {
    SkASSERT(fMapPtr && this->isMappable());

    SkASSERT(fAlloc.fSize > 0);
    SkASSERT(fAlloc.fSize >= flushOffset + flushSize);

    auto allocator = this->vulkanSharedContext()->memoryAllocator();
    skgpu::VulkanMemory::FlushMappedAlloc(allocator, fAlloc, flushOffset, flushSize, nullptr);
    skgpu::VulkanMemory::UnmapAlloc(allocator, fAlloc);
}

void VulkanBuffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());

    this->internalMap(0, fBufferUsedForCPURead ? this->size() : 0);
}

void VulkanBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
    this->internalUnmap(0, fBufferUsedForCPURead ? 0 : this->size());
}
} // namespace skgpu::graphite

