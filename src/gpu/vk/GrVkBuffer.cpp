/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkBuffer.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkTransferBuffer.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

using BufferUsage = GrVkMemoryAllocator::BufferUsage;

static BufferUsage get_buffer_usage(GrVkBuffer::Type type, bool dynamic) {
    switch (type) {
        case GrVkBuffer::kVertex_Type:    // fall through
        case GrVkBuffer::kIndex_Type:     // fall through
        case GrVkBuffer::kIndirect_Type:  // fall through
        case GrVkBuffer::kTexel_Type:
            return dynamic ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
        case GrVkBuffer::kUniform_Type:  // fall through
        case GrVkBuffer::kCopyRead_Type:
            SkASSERT(dynamic);
            return BufferUsage::kCpuWritesGpuReads;
        case GrVkBuffer::kCopyWrite_Type:
            return BufferUsage::kGpuWritesCpuReads;
    }
    SK_ABORT("Invalid GrVkBuffer::Type");
}

const GrVkBuffer::Resource* GrVkBuffer::Create(GrVkGpu* gpu, const Desc& desc) {
    SkASSERT(!gpu->protectedContext() || (gpu->protectedContext() == desc.fDynamic));
    VkBuffer       buffer;
    GrVkAlloc      alloc;

    // create the buffer object
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = 0;
    bufInfo.size = desc.fSizeInBytes;
    switch (desc.fType) {
        case kVertex_Type:
            bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case kIndex_Type:
            bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case kIndirect_Type:
            bufInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            break;
        case kUniform_Type:
            bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case kCopyRead_Type:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case kCopyWrite_Type:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case kTexel_Type:
            bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (!desc.fDynamic) {
        bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;

    VkResult err;
    err = VK_CALL(gpu, CreateBuffer(gpu->device(), &bufInfo, nullptr, &buffer));
    if (err) {
        return nullptr;
    }

    if (!GrVkMemory::AllocAndBindBufferMemory(gpu,
                                              buffer,
                                              get_buffer_usage(desc.fType, desc.fDynamic),
                                              &alloc)) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        return nullptr;
    }

    const GrVkBuffer::Resource* resource = new GrVkBuffer::Resource(gpu, buffer, alloc);
    if (!resource) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        GrVkMemory::FreeBufferMemory(gpu, alloc);
        return nullptr;
    }

    return resource;
}

void GrVkBuffer::addMemoryBarrier(const GrVkGpu* gpu,
                                  VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccesMask,
                                  VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask,
                                  bool byRegion) const {
    VkBufferMemoryBarrier bufferMemoryBarrier = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, // sType
        nullptr,                                 // pNext
        srcAccessMask,                           // srcAccessMask
        dstAccesMask,                            // dstAccessMask
        VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
        this->buffer(),                          // buffer
        0,                                       // offset
        fDesc.fSizeInBytes,                      // size
    };

    // TODO: restrict to area of buffer we're interested in
    gpu->addBufferMemoryBarrier(this->resource(), srcStageMask, dstStageMask, byRegion,
                                &bufferMemoryBarrier);
}

void GrVkBuffer::Resource::freeGPUData() const {
    SkASSERT(fBuffer);
    SkASSERT(fAlloc.fMemory);
    VK_CALL(fGpu, DestroyBuffer(fGpu->device(), fBuffer, nullptr));
    GrVkMemory::FreeBufferMemory(fGpu, fAlloc);
}

void GrVkBuffer::vkRelease(GrVkGpu* gpu) {
    VALIDATE();
    if (this->vkIsMapped()) {
        // Only unmap resources that are not backed by a CPU buffer. Otherwise we may end up
        // creating a new transfer buffer resources that sends us into a spiral of creating and
        // destroying resources if we are at our budget limit. Also there really isn't a need to
        // upload the CPU data if we are deleting this buffer.
        if (fDesc.fDynamic) {
            this->vkUnmap(gpu);
        }
    }
    fResource->recycle();
    fResource = nullptr;
    if (!fDesc.fDynamic) {
        delete[] (unsigned char*)fMapPtr;
    }
    fMapPtr = nullptr;
    VALIDATE();
}

VkAccessFlags buffer_type_to_access_flags(GrVkBuffer::Type type) {
    switch (type) {
        case GrVkBuffer::kIndex_Type:
            return VK_ACCESS_INDEX_READ_BIT;
        case GrVkBuffer::kVertex_Type:
            return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        default:
            // This helper is only called for static buffers so we should only ever see index or
            // vertex buffers types
            SkASSERT(false);
            return 0;
    }
}

void GrVkBuffer::internalMap(GrVkGpu* gpu, size_t size, bool* createdNewBuffer) {
    VALIDATE();
    SkASSERT(!this->vkIsMapped());

    if (!fResource->unique()) {
        SkASSERT(fDesc.fDynamic);
        // in use by the command buffer, so we need to create a new one
        fResource->recycle();
        fResource = this->createResource(gpu, fDesc);
        if (createdNewBuffer) {
            *createdNewBuffer = true;
        }
    }

    if (fDesc.fDynamic) {
        const GrVkAlloc& alloc = this->alloc();
        SkASSERT(alloc.fSize > 0);
        SkASSERT(alloc.fSize >= size);
        SkASSERT(0 == fOffset);

        fMapPtr = GrVkMemory::MapAlloc(gpu, alloc);
    } else {
        SkASSERT(!fMapPtr);
        fMapPtr = new unsigned char[this->size()];
    }

    VALIDATE();
}

void GrVkBuffer::copyCpuDataToGpuBuffer(GrVkGpu* gpu, const void* src, size_t size) {
    SkASSERT(src);
    // We should never call this method in protected contexts.
    SkASSERT(!gpu->protectedContext());
    // The vulkan api restricts the use of vkCmdUpdateBuffer to updates that are less than or equal
    // to 65536 bytes and a size the is 4 byte aligned.
    if ((size <= 65536) && (0 == (size & 0x3)) && !gpu->vkCaps().avoidUpdateBuffers()) {
        gpu->updateBuffer(this, src, this->offset(), size);
    } else {
        sk_sp<GrVkTransferBuffer> transferBuffer =
                GrVkTransferBuffer::Make(gpu, size, GrVkBuffer::kCopyRead_Type,
                                         kStream_GrAccessPattern);
        if (!transferBuffer) {
            return;
        }

        char* buffer = (char*) transferBuffer->map();
        memcpy (buffer, src, size);
        transferBuffer->unmap();

        gpu->copyBuffer(transferBuffer.get(), this, 0, this->offset(), size);
    }
    this->addMemoryBarrier(gpu,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           buffer_type_to_access_flags(fDesc.fType),
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           false);
}

void GrVkBuffer::internalUnmap(GrVkGpu* gpu, size_t size) {
    VALIDATE();
    SkASSERT(this->vkIsMapped());

    if (fDesc.fDynamic) {
        const GrVkAlloc& alloc = this->alloc();
        SkASSERT(alloc.fSize > 0);
        SkASSERT(alloc.fSize >= size);
        // We currently don't use fOffset
        SkASSERT(0 == fOffset);

        GrVkMemory::FlushMappedAlloc(gpu, alloc, 0, size);
        GrVkMemory::UnmapAlloc(gpu, alloc);
        fMapPtr = nullptr;
    } else {
        SkASSERT(fMapPtr);
        this->copyCpuDataToGpuBuffer(gpu, fMapPtr, size);
    }
}

bool GrVkBuffer::vkIsMapped() const {
    VALIDATE();
    return SkToBool(fMapPtr);
}

bool GrVkBuffer::vkUpdateData(GrVkGpu* gpu, const void* src, size_t srcSizeInBytes,
                              bool* createdNewBuffer) {
    if (srcSizeInBytes > fDesc.fSizeInBytes) {
        return false;
    }

    if (fDesc.fDynamic) {
        this->internalMap(gpu, srcSizeInBytes, createdNewBuffer);
        if (!fMapPtr) {
            return false;
        }

        memcpy(fMapPtr, src, srcSizeInBytes);
        this->internalUnmap(gpu, srcSizeInBytes);
    } else {
        this->copyCpuDataToGpuBuffer(gpu, src, srcSizeInBytes);
    }


    return true;
}

void GrVkBuffer::validate() const {
    SkASSERT(!fResource || kVertex_Type == fDesc.fType || kIndex_Type == fDesc.fType ||
             kIndirect_Type == fDesc.fType || kTexel_Type == fDesc.fType ||
             kCopyRead_Type == fDesc.fType || kCopyWrite_Type == fDesc.fType ||
             kUniform_Type == fDesc.fType);
}
