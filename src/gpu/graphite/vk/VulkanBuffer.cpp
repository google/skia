/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanBuffer.h"

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
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

    // TODO (b/374749633): We can't use protected buffers in the vertex shader. The checks below
    // make sure we don't use it for vertex or index buffers. But we currently don't have a way to
    // check here if it is a uniform or storage buffer that is used in the vertex shader. If we hit
    // that issue and need those GpuOnly buffers, we'll need to pass in some information to the
    // factory to say what stage the buffer is for. Maybe expand AccessPattern to be
    // GpuOnly_NotVertex or some better name like that.
    bool isProtected = sharedContext->isProtected() == Protected::kYes &&
                       (accessPattern == AccessPattern::kGpuOnly ||
                        accessPattern == AccessPattern::kGpuOnlyCopySrc) &&
                       type != BufferType::kVertex &&
                       type != BufferType::kIndex;

    // kGpuOnlyCopySrc is used during testing to overwrite buffer accessPatterns that would normally
    // be AccessPattern::kGpuOnly. So make sure that buffers that *should* be protected, don't
    // accidentally expose access here.
    SkASSERT(!isProtected || accessPattern != AccessPattern::kGpuOnlyCopySrc);

    // Protected memory _never_ uses mappable buffers.
    // Otherwise, the only time we don't require mappable buffers is when we're on a device
    // where gpu only memory has faster reads on the gpu than memory that is also mappable
    // on the cpu.
    bool requiresMappable = !isProtected &&
                            (accessPattern == AccessPattern::kHostVisible ||
                             !sharedContext->vulkanCaps().gpuOnlyBuffersMorePerformant());

    using BufferUsage = skgpu::VulkanMemoryAllocator::BufferUsage;

    BufferUsage allocUsage;
    if (type == BufferType::kXferCpuToGpu) {
        allocUsage = BufferUsage::kTransfersFromCpuToGpu;
    } else if (type == BufferType::kXferGpuToCpu) {
        allocUsage = BufferUsage::kTransfersFromGpuToCpu;
    } else {
        // GPU-only buffers are preferred unless mappability is required.
        allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
    }

    // Create the buffer object
    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = isProtected ? VK_BUFFER_CREATE_PROTECTED_BIT : 0;
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
        case BufferType::kQuery:
            SK_ABORT("Query buffers not supported on Vulkan");
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
            break;
        case BufferType::kXferCpuToGpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case BufferType::kXferGpuToCpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
    }

    // We may not always get a mappable buffer for non-dynamic access buffers. Thus we set the
    // transfer dst usage bit in case we need to do a copy to write data. It doesn't really hurt
    // to set this extra usage flag, but we could narrow the scope of buffers we set it on more than
    // just not dynamic.
    if (!requiresMappable || accessPattern == AccessPattern::kGpuOnly ||
        accessPattern == AccessPattern::kGpuOnlyCopySrc) {
        bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (accessPattern == AccessPattern::kGpuOnlyCopySrc) {
        bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;

    VkResult result;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateBuffer(sharedContext->device(),
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
                                                skgpu::Protected(isProtected),
                                                allocUsage,
                                                shouldPersistentlyMapCpuToGpu,
                                                checkResult,
                                                &alloc)) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyBuffer(sharedContext->device(),
                                  buffer,
                                  /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    // Bind buffer
    VULKAN_CALL_RESULT(
            sharedContext,
            result,
            BindBufferMemory(sharedContext->device(), buffer, alloc.fMemory, alloc.fOffset));
    if (result != VK_SUCCESS) {
        skgpu::VulkanMemory::FreeBufferMemory(allocator, alloc);
        VULKAN_CALL(sharedContext->interface(), DestroyBuffer(sharedContext->device(),
                buffer,
                /*const VkAllocationCallbacks*=*/nullptr));
        return nullptr;
    }

    return sk_sp<Buffer>(new VulkanBuffer(
            sharedContext, size, type, accessPattern, std::move(buffer), alloc, bufInfo.usage,
            Protected(isProtected)));
}

VulkanBuffer::VulkanBuffer(const VulkanSharedContext* sharedContext,
                           size_t size,
                           BufferType type,
                           AccessPattern accessPattern,
                           VkBuffer buffer,
                           const skgpu::VulkanAlloc& alloc,
                           const VkBufferUsageFlags usageFlags,
                           Protected isProtected)
        : Buffer(sharedContext, size, isProtected)
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
        SkASSERT(fAlloc.fSize > 0);
        SkASSERT(fAlloc.fSize >= readOffset + readSize);

        const VulkanSharedContext* sharedContext = this->vulkanSharedContext();

        auto allocator = sharedContext->memoryAllocator();
        auto checkResult = [sharedContext](VkResult result) {
            VULKAN_LOG_IF_NOT_SUCCESS(sharedContext, result, "skgpu::VulkanMemory::MapAlloc");
            return sharedContext->checkVkResult(result);
        };
        fMapPtr = skgpu::VulkanMemory::MapAlloc(allocator, fAlloc, checkResult);
        if (fMapPtr && readSize != 0) {
            auto checkResult_invalidate = [sharedContext, readOffset, readSize](VkResult result) {
                VULKAN_LOG_IF_NOT_SUCCESS(sharedContext,
                                          result,
                                          "skgpu::VulkanMemory::InvalidateMappedAlloc "
                                          "(readOffset:%zu, readSize:%zu)",
                                          readOffset,
                                          readSize);
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
        VULKAN_LOG_IF_NOT_SUCCESS(sharedContext,
                                  result,
                                  "skgpu::VulkanMemory::FlushMappedAlloc "
                                  "(flushOffset:%zu, flushSize:%zu)",
                                  flushOffset,
                                  flushSize);
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

namespace {

VkPipelineStageFlags access_to_pipeline_srcStageFlags(const VkAccessFlags srcAccess) {
    // For now this function assumes the access flags equal a specific bit and don't act like true
    // flags (i.e. set of bits). If we ever start having buffer usages that have multiple accesses
    // in one usage we'll need to update this.
    switch (srcAccess) {
        case 0:
            return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        case (VK_ACCESS_TRANSFER_WRITE_BIT):  // fallthrough
        case (VK_ACCESS_TRANSFER_READ_BIT):
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case (VK_ACCESS_UNIFORM_READ_BIT):
            // TODO(b/307577875): It is possible that uniforms could have simply been used in the
            // vertex shader and not the fragment shader, so using the fragment shader pipeline
            // stage bit indiscriminately is a bit overkill. This call should be modified to check &
            // allow for selecting VK_PIPELINE_STAGE_VERTEX_SHADER_BIT when appropriate.
            return (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        case (VK_ACCESS_SHADER_WRITE_BIT):
            return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case (VK_ACCESS_INDEX_READ_BIT):  // fallthrough
        case (VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT):
            return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        case (VK_ACCESS_INDIRECT_COMMAND_READ_BIT):
            return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        case (VK_ACCESS_HOST_READ_BIT):  // fallthrough
        case (VK_ACCESS_HOST_WRITE_BIT):
            return VK_PIPELINE_STAGE_HOST_BIT;
        default:
            SkUNREACHABLE;
    }
}

bool access_is_read_only(VkAccessFlags access) {
    switch (access) {
        case 0: // initialization state
        case (VK_ACCESS_TRANSFER_READ_BIT):
        case (VK_ACCESS_UNIFORM_READ_BIT):
        case (VK_ACCESS_INDEX_READ_BIT):
        case (VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT):
        case (VK_ACCESS_INDIRECT_COMMAND_READ_BIT):
        case (VK_ACCESS_HOST_READ_BIT):
            return true;
        case (VK_ACCESS_TRANSFER_WRITE_BIT):
        case (VK_ACCESS_SHADER_WRITE_BIT):
        case (VK_ACCESS_HOST_WRITE_BIT):
            return false;
        default:
            SkUNREACHABLE;
    }
}

} // anonymous namespace

void VulkanBuffer::setBufferAccess(VulkanCommandBuffer* cmdBuffer,
                                   VkAccessFlags dstAccess,
                                   VkPipelineStageFlags dstStageMask) const {
    SkASSERT(dstAccess == VK_ACCESS_HOST_READ_BIT ||
             dstAccess == VK_ACCESS_TRANSFER_WRITE_BIT ||
             dstAccess == VK_ACCESS_TRANSFER_READ_BIT ||
             dstAccess == VK_ACCESS_UNIFORM_READ_BIT ||
             dstAccess == VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ||
             dstAccess == VK_ACCESS_INDEX_READ_BIT);

    VkPipelineStageFlags srcStageMask = access_to_pipeline_srcStageFlags(fCurrentAccess);
    SkASSERT(srcStageMask);

    bool needsBarrier = true;

    // When the buffer was last used on the host, we don't need to add any barrier as writes on the
    // CPU host are implicitly synchronized what you submit new commands.
    if (srcStageMask == VK_PIPELINE_STAGE_HOST_BIT) {
        needsBarrier = false;
    } else if (access_is_read_only(fCurrentAccess) && access_is_read_only(dstAccess)) {
        // We don't need a barrier if we're going from a read access to another read access, and we
        // have the same type of read only access.
        if (fCurrentAccess == dstAccess) {
            needsBarrier = false;
        } else {
            /*needBarrier=true*/
            // NOTE: Currently this setup *only* correctly handles copying from a vertex
            // kGpuOnlyCopySrc buffer to kXferGpuToCpu buffer for read backs during testing on
            // non-protected data.
            //
            // In the future we'll need to update the logic in this file to store all the read
            // accesses in a mask. Additionally we'll need to keep track of what the last write was
            // since we will need to add a barrier for the new read access--even if we have to put
            // in a barrier for a previous read already. For example if we have the sequence
            // Write_1, Read_Access1, Read_Access2. We will first put a barrier going from Write_1
            // to Read_Access1. But with the current logic when we add Read_Access2 it will think
            // its going from a read -> read. Thus no barrier would be added. But we need do to add
            // another barrier for Write_1 to Read_Access2 so that the changes from write become
            // visibile.
        }
    }

    if (needsBarrier) {
        VkBufferMemoryBarrier bufferMemoryBarrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType
            nullptr,                                  // pNext
            fCurrentAccess,                           // srcAccessMask
            dstAccess,                                // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            fBuffer,                                  // buffer
            0,                                        // offset
            this->size(),                             // size
        };
        cmdBuffer->addBufferMemoryBarrier(srcStageMask, dstStageMask, &bufferMemoryBarrier);
    }

    fCurrentAccess = dstAccess;
}

} // namespace skgpu::graphite
