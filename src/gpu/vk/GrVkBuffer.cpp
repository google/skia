/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkBuffer.h"
#include "GrVkGpu.h"
#include "GrVkMemory.h"
#include "GrVkTransferBuffer.h"
#include "GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

const GrVkBuffer::Resource* GrVkBuffer::Create(const GrVkGpu* gpu, const Desc& desc) {
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
                                              desc.fType,
                                              desc.fDynamic,
                                              &alloc)) {
        return nullptr;
    }

    const GrVkBuffer::Resource* resource = new GrVkBuffer::Resource(buffer, alloc, desc.fType);
    if (!resource) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        GrVkMemory::FreeBufferMemory(gpu, desc.fType, alloc);
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
    gpu->addBufferMemoryBarrier(srcStageMask, dstStageMask, byRegion, &bufferMemoryBarrier);
}

void GrVkBuffer::Resource::freeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(fBuffer);
    SkASSERT(fAlloc.fMemory);
    VK_CALL(gpu, DestroyBuffer(gpu->device(), fBuffer, nullptr));
    GrVkMemory::FreeBufferMemory(gpu, fType, fAlloc);
}

void GrVkBuffer::vkRelease(const GrVkGpu* gpu) {
    VALIDATE();
    fResource->recycle(const_cast<GrVkGpu*>(gpu));
    fResource = nullptr;
    if (!fDesc.fDynamic) {
        delete[] (unsigned char*)fMapPtr;
    }
    fMapPtr = nullptr;
    VALIDATE();
}

void GrVkBuffer::vkAbandon() {
    fResource->unrefAndAbandon();
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
        if (fDesc.fDynamic) {
            // in use by the command buffer, so we need to create a new one
            fResource->recycle(gpu);
            fResource = this->createResource(gpu, fDesc);
            if (createdNewBuffer) {
                *createdNewBuffer = true;
            }
        } else {
            SkASSERT(fMapPtr);
            this->addMemoryBarrier(gpu,
                                   buffer_type_to_access_flags(fDesc.fType),
                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   false);
        }
    }

    if (fDesc.fDynamic) {
        const GrVkAlloc& alloc = this->alloc();
        SkASSERT(alloc.fSize > 0);

        // For Noncoherent buffers we want to make sure the range that we map, both offset and size,
        // are aligned to the nonCoherentAtomSize limit. The offset should have been correctly
        // aligned by our memory allocator. For size we pad out to make the range also aligned.
        if (SkToBool(alloc.fFlags & GrVkAlloc::kNoncoherent_Flag)) {
            // Currently we always have the internal offset as 0.
            SkASSERT(0 == fOffset);
            VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
            SkASSERT(0 == (alloc.fOffset & (alignment - 1)));

            // Make size of the map aligned to nonCoherentAtomSize
            size = (size + alignment - 1) & ~(alignment - 1);
            fMappedSize = size;
        }
        SkASSERT(size + fOffset <= alloc.fSize);
        VkResult err = VK_CALL(gpu, MapMemory(gpu->device(), alloc.fMemory,
                                              alloc.fOffset + fOffset,
                                              size, 0, &fMapPtr));
        if (err) {
            fMapPtr = nullptr;
            fMappedSize = 0;
        }
    } else {
        if (!fMapPtr) {
            fMapPtr = new unsigned char[this->size()];
        }
    }

    VALIDATE();
}

void GrVkBuffer::internalUnmap(GrVkGpu* gpu, size_t size) {
    VALIDATE();
    SkASSERT(this->vkIsMapped());

    if (fDesc.fDynamic) {
        // We currently don't use fOffset
        SkASSERT(0 == fOffset);
        VkDeviceSize flushOffset = this->alloc().fOffset + fOffset;
        VkDeviceSize flushSize = gpu->vkCaps().canUseWholeSizeOnFlushMappedMemory() ? VK_WHOLE_SIZE
                                                                                    : fMappedSize;

        GrVkMemory::FlushMappedAlloc(gpu, this->alloc(), flushOffset, flushSize);
        VK_CALL(gpu, UnmapMemory(gpu->device(), this->alloc().fMemory));
        fMapPtr = nullptr;
        fMappedSize = 0;
    } else {
        // vkCmdUpdateBuffer requires size < 64k and 4-byte alignment.
        // https://bugs.chromium.org/p/skia/issues/detail?id=7488
        if (size <= 65536 && 0 == (size & 0x3)) {
            gpu->updateBuffer(this, fMapPtr, this->offset(), size);
        } else {
            GrVkTransferBuffer* transferBuffer =
                    GrVkTransferBuffer::Create(gpu, size, GrVkBuffer::kCopyRead_Type);
            if(!transferBuffer) {
                return;
            }

            char* buffer = (char*) transferBuffer->map();
            memcpy (buffer, fMapPtr, size);
            transferBuffer->unmap();

            gpu->copyBuffer(transferBuffer, this, 0, this->offset(), size);
            transferBuffer->unref();
        }
        this->addMemoryBarrier(gpu,
                               VK_ACCESS_TRANSFER_WRITE_BIT,
                               buffer_type_to_access_flags(fDesc.fType),
                               VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                               false);
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

    this->internalMap(gpu, srcSizeInBytes, createdNewBuffer);
    if (!fMapPtr) {
        return false;
    }

    memcpy(fMapPtr, src, srcSizeInBytes);

    this->internalUnmap(gpu, srcSizeInBytes);

    return true;
}

void GrVkBuffer::validate() const {
    SkASSERT(!fResource || kVertex_Type == fDesc.fType || kIndex_Type == fDesc.fType
             || kTexel_Type == fDesc.fType || kCopyRead_Type == fDesc.fType
             || kCopyWrite_Type == fDesc.fType || kUniform_Type == fDesc.fType);
}
