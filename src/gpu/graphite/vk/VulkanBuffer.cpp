/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanBuffer.h"

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/vk/VulkanMemory.h"

namespace skgpu::graphite {

sk_sp<Buffer> VulkanBuffer::Make(const VulkanSharedContext* sharedContext,
                                 size_t size,
                                 BufferType type,
                                 AccessPattern accessPattern) {
    if (size <= 0) {
        return nullptr;
    }
    VkBuffer buffer;
    skgpu::VulkanAlloc alloc;

    // The only time we don't require mappable buffers is when we're on a device where gpu only
    // memory has faster reads on the gpu than memory that is also mappable on the cpu. Protected
    // memory always uses mappable buffers.
    bool requiresMappable = sharedContext->isProtected() == Protected::kYes ||
                            accessPattern == AccessPattern::kHostVisible ||
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
    // dsts. To support rtAdjust uniform buffer updates, we make host-visible uniform buffers also
    // capable of being transfer dsts.
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
    if (!requiresMappable || accessPattern == AccessPattern::kGpuOnly) {
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

    return sk_sp<Buffer>(new VulkanBuffer(
            sharedContext, size, type, accessPattern, std::move(buffer), alloc, bufInfo.usage));
}

VulkanBuffer::VulkanBuffer(const VulkanSharedContext* sharedContext,
                           size_t size,
                           BufferType type,
                           AccessPattern accessPattern,
                           VkBuffer buffer,
                           const skgpu::VulkanAlloc& alloc,
                           const VkBufferUsageFlags usageFlags)
        : Buffer(sharedContext, size)
        , fBuffer(std::move(buffer))
        , fAlloc(alloc)
        , fBufferUsageFlags(usageFlags)
        // We assume a buffer is used for CPU reads only in the case of GPU->CPU transfer buffers.
        , fBufferUsedForCPURead(type == BufferType::kXferGpuToCpu) {}

void VulkanBuffer::freeGpuData() {
    if (fMapPtr) {
        this->internalUnmap(0, this->size());
        fMapPtr = nullptr;
    }

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
            VULKAN_LOG_IF_NOT_SUCCESS(result, "skgpu::VulkanMemory::MapAlloc");
            return sharedContext->checkVkResult(result);
        };
        fMapPtr = skgpu::VulkanMemory::MapAlloc(allocator, fAlloc, checkResult);
        if (fMapPtr && readSize != 0) {
            auto checkResult_invalidate = [sharedContext, readOffset, readSize](VkResult result) {
                VULKAN_LOG_IF_NOT_SUCCESS(result, "skgpu::VulkanMemory::InvalidateMappedAlloc "
                                                  "(readOffset:%zu, readSize:%zu)",
                                          readOffset, readSize);
                return sharedContext->checkVkResult(result);
            };
            // "Invalidate" here means make device writes visible to the host. That is, it makes
            // sure any GPU writes are finished in the range we might read from.
            skgpu::VulkanMemory::InvalidateMappedAlloc(allocator,
                                                       fAlloc,
                                                       readOffset,
                                                       readSize,
                                                       checkResult_invalidate);
        }
    }
}

void VulkanBuffer::internalUnmap(size_t flushOffset, size_t flushSize) {
    SkASSERT(fMapPtr && this->isMappable());

    SkASSERT(fAlloc.fSize > 0);
    SkASSERT(fAlloc.fSize >= flushOffset + flushSize);

    const VulkanSharedContext* sharedContext = this->vulkanSharedContext();
    auto checkResult = [sharedContext, flushOffset, flushSize](VkResult result) {
       VULKAN_LOG_IF_NOT_SUCCESS(result, "skgpu::VulkanMemory::FlushMappedAlloc "
                                         "(flushOffset:%zu, flushSize:%zu)",
                                         flushOffset, flushSize);
        return sharedContext->checkVkResult(result);
    };

    auto allocator = sharedContext->memoryAllocator();
    skgpu::VulkanMemory::FlushMappedAlloc(allocator, fAlloc, flushOffset, flushSize, checkResult);
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

void VulkanBuffer::setBufferAccess(VulkanCommandBuffer* cmdBuffer,
                                   VkAccessFlags dstAccessMask,
                                   VkPipelineStageFlags dstStageMask) const {
    // TODO: fill out other cases where we need a barrier
    if (dstAccessMask == VK_ACCESS_HOST_READ_BIT      ||
        dstAccessMask == VK_ACCESS_TRANSFER_WRITE_BIT ||
        dstAccessMask == VK_ACCESS_UNIFORM_READ_BIT) {
        VkPipelineStageFlags srcStageMask =
            VulkanBuffer::AccessMaskToPipelineSrcStageFlags(fCurrentAccessMask);

        VkBufferMemoryBarrier bufferMemoryBarrier = {
                 VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType
                 nullptr,                                  // pNext
                 fCurrentAccessMask,                       // srcAccessMask
                 dstAccessMask,                            // dstAccessMask
                 VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
                 VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
                 fBuffer,                                  // buffer
                 0,                                        // offset
                 this->size(),                             // size
        };
        cmdBuffer->addBufferMemoryBarrier(srcStageMask, dstStageMask, &bufferMemoryBarrier);
    }

    fCurrentAccessMask = dstAccessMask;
}

VkPipelineStageFlags VulkanBuffer::AccessMaskToPipelineSrcStageFlags(const VkAccessFlags srcMask) {
    if (srcMask == 0) {
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    VkPipelineStageFlags flags = 0;

    if (srcMask & VK_ACCESS_TRANSFER_WRITE_BIT || srcMask & VK_ACCESS_TRANSFER_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (srcMask & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ||
        srcMask & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (srcMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ||
        srcMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if (srcMask & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (srcMask & VK_ACCESS_SHADER_READ_BIT ||
        srcMask & VK_ACCESS_UNIFORM_READ_BIT) {
        // TODO(b/307577875): It is possible that uniforms could have simply been used in the vertex
        // shader and not the fragment shader, so using the fragment shader pipeline stage bit
        // indiscriminately is a bit overkill. This call should be modified to check & allow for
        // selecting VK_PIPELINE_STAGE_VERTEX_SHADER_BIT when appropriate.
        flags |= (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    if (srcMask & VK_ACCESS_SHADER_WRITE_BIT) {
        flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if (srcMask & VK_ACCESS_INDEX_READ_BIT ||
        srcMask & VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (srcMask & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) {
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }
    if (srcMask & VK_ACCESS_HOST_READ_BIT || srcMask & VK_ACCESS_HOST_WRITE_BIT) {
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    }

    return flags;
}

} // namespace skgpu::graphite
