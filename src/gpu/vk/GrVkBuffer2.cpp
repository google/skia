/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkBuffer2.h"

#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkTransferBuffer.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkBuffer2::GrVkBuffer2(GrVkGpu* gpu,
                         size_t sizeInBytes,
                         GrGpuBufferType bufferType,
                         GrAccessPattern accessPattern,
                         VkBuffer buffer,
                         const GrVkAlloc& alloc,
                         const GrVkDescriptorSet* uniformDescriptorSet)
        : GrGpuBuffer(gpu, sizeInBytes, bufferType, accessPattern)
        , fBuffer(buffer)
        , fAlloc(alloc)
        , fUniformDescriptorSet(uniformDescriptorSet) {
    // We always require dynamic buffers to be mappable
    SkASSERT(accessPattern != kDynamic_GrAccessPattern || this->isVkMappable());
    SkASSERT(bufferType != GrGpuBufferType::kUniform || uniformDescriptorSet);
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrVkBuffer2> GrVkBuffer2::MakeUniform(GrVkGpu* gpu, size_t size) {
    return Make(gpu, size, GrGpuBufferType::kUniform, kDynamic_GrAccessPattern);
}

static const GrVkDescriptorSet* make_uniform_desc_set(GrVkGpu* gpu, VkBuffer buffer, size_t size) {
    const GrVkDescriptorSet* descriptorSet = gpu->resourceProvider().getUniformDescriptorSet();
    if (!descriptorSet) {
        return nullptr;
    }

    VkDescriptorBufferInfo bufferInfo;
    memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWrite;
    memset(&descriptorWrite, 0, sizeof(VkWriteDescriptorSet));
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.pNext = nullptr;
    descriptorWrite.dstSet = *descriptorSet->descriptorSet();
    descriptorWrite.dstBinding = GrVkUniformHandler::kUniformBinding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(),
               UpdateDescriptorSets(gpu->device(), 1, &descriptorWrite, 0, nullptr));
    return descriptorSet;
}

sk_sp<GrVkBuffer2> GrVkBuffer2::Make(GrVkGpu* gpu,
                                     size_t size,
                                     GrGpuBufferType bufferType,
                                     GrAccessPattern accessPattern) {
    VkBuffer buffer;
    GrVkAlloc alloc;

    bool requiresMappable = gpu->protectedContext() ||
                            accessPattern == kDynamic_GrAccessPattern ||
                            accessPattern == kStream_GrAccessPattern ||
                            !gpu->vkCaps().gpuOnlyBuffersMorePerformant();

    using BufferUsage = GrVkMemoryAllocator::BufferUsage;
    BufferUsage allocUsage;

    // create the buffer object
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = 0;
    bufInfo.size = size;
    switch (bufferType) {
        case GrGpuBufferType::kVertex:
            bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
            break;
        case GrGpuBufferType::kIndex:
            bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
            break;
        case GrGpuBufferType::kDrawIndirect:
            bufInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            allocUsage = requiresMappable ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
            break;
        case GrGpuBufferType::kUniform:
            bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            allocUsage = BufferUsage::kCpuWritesGpuReads;
            break;
        case GrGpuBufferType::kXferCpuToGpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocUsage = BufferUsage::kCpuWritesGpuReads;
            break;
        case GrGpuBufferType::kXferGpuToCpu:
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocUsage = BufferUsage::kGpuWritesCpuReads;
            break;
    }
    // We may not always get a mappable buffer for non dynamic access buffers. Thus we set the
    // transfer dst usage bit in case we need to do a copy to write data.
    // TODO: It doesn't really hurt setting this extra usage flag, but maybe we can narrow the scope
    // of buffers we set it on more than just not dynamic.
    if (!requiresMappable) {
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

    if (!GrVkMemory::AllocAndBindBufferMemory(gpu, buffer, allocUsage, &alloc)) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        return nullptr;
    }

    // If this is a uniform buffer we must setup a descriptor set
    const GrVkDescriptorSet* uniformDescSet = nullptr;
    if (bufferType == GrGpuBufferType::kUniform) {
        uniformDescSet = make_uniform_desc_set(gpu, buffer, size);
        if (!uniformDescSet) {
            VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
            GrVkMemory::FreeBufferMemory(gpu, alloc);
            return nullptr;
        }
    }

    return sk_sp<GrVkBuffer2>(new GrVkBuffer2(gpu, size, bufferType, accessPattern, buffer, alloc,
                                              uniformDescSet));
}

void GrVkBuffer2::vkMap(size_t size) {
    SkASSERT(!fMapPtr);
    if (this->isVkMappable()) {
        // Not every buffer will use command buffer usage refs and instead the command buffer just
        // holds normal refs. Systems higher up in Ganesh should be making sure not to reuse a
        // buffer that currently has a ref held by something else. However, we do need to make sure
        // there isn't a buffer with just a command buffer usage that is trying to be mapped.
        SkASSERT(this->internalHasNoCommandBufferUsages());
        SkASSERT(fAlloc.fSize > 0);
        SkASSERT(fAlloc.fSize >= size);
        fMapPtr = GrVkMemory::MapAlloc(this->getVkGpu(), fAlloc);
    }
}

void GrVkBuffer2::vkUnmap(size_t size) {
    SkASSERT(fMapPtr && this->isVkMappable());

    SkASSERT(fAlloc.fSize > 0);
    SkASSERT(fAlloc.fSize >= size);

    GrVkGpu* gpu = this->getVkGpu();
    GrVkMemory::FlushMappedAlloc(gpu, fAlloc, 0, size);
    GrVkMemory::UnmapAlloc(gpu, fAlloc);
}

static VkAccessFlags buffer_type_to_access_flags(GrGpuBufferType type) {
    switch (type) {
        case GrGpuBufferType::kIndex:
            return VK_ACCESS_INDEX_READ_BIT;
        case GrGpuBufferType::kVertex:
            return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        default:
            // This helper is only called for static buffers so we should only ever see index or
            // vertex buffers types
            SkUNREACHABLE;
    }
}

void GrVkBuffer2::copyCpuDataToGpuBuffer(const void* src, size_t size) {
    SkASSERT(src);

    GrVkGpu* gpu = this->getVkGpu();

    // We should never call this method in protected contexts.
    SkASSERT(!gpu->protectedContext());

    // The vulkan api restricts the use of vkCmdUpdateBuffer to updates that are less than or equal
    // to 65536 bytes and a size the is 4 byte aligned.
    if ((size <= 65536) && (0 == (size & 0x3)) && !gpu->vkCaps().avoidUpdateBuffers()) {
        gpu->updateBuffer(sk_ref_sp(this), src, /*offset=*/0, size);
    } else {
        sk_sp<GrVkTransferBuffer> transferBuffer =
                GrVkTransferBuffer::Make(gpu, size, GrVkBuffer::kCopyRead_Type,
                                         kStream_GrAccessPattern);
        if (!transferBuffer) {
            return;
        }

        char* buffer = (char*)transferBuffer->map();
        memcpy(buffer, src, size);
        transferBuffer->unmap();

        gpu->copyBuffer(transferBuffer.get(), sk_ref_sp(this), /*srcOffset=*/0, /*dstOffset=*/0,
                        size);
    }

    this->addMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT,
                           buffer_type_to_access_flags(this->intendedType()),
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           /*byRegion=*/false);
}

void GrVkBuffer2::addMemoryBarrier(VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccesMask,
                                  VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask,
                                  bool byRegion) const {
    VkBufferMemoryBarrier bufferMemoryBarrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType
            nullptr,                                  // pNext
            srcAccessMask,                            // srcAccessMask
            dstAccesMask,                             // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            fBuffer,                                  // buffer
            0,                                        // offset
            this->size(),                             // size
    };

    // TODO: restrict to area of buffer we're interested in
    this->getVkGpu()->addBufferMemoryBarrier(srcStageMask, dstStageMask, byRegion,
                                             &bufferMemoryBarrier);
}

void GrVkBuffer2::vkRelease() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMapPtr) {
        this->vkUnmap(this->size());
        fMapPtr = nullptr;
    }

    if (fUniformDescriptorSet) {
        fUniformDescriptorSet->recycle();
        fUniformDescriptorSet = nullptr;
    }

    SkASSERT(fBuffer);
    SkASSERT(fAlloc.fMemory && fAlloc.fBackendMemory);
    VK_CALL(this->getVkGpu(), DestroyBuffer(this->getVkGpu()->device(), fBuffer, nullptr));
    fBuffer = VK_NULL_HANDLE;

    GrVkMemory::FreeBufferMemory(this->getVkGpu(), fAlloc);
    fAlloc.fMemory = VK_NULL_HANDLE;
    fAlloc.fBackendMemory = 0;
}

void GrVkBuffer2::onRelease() {
    this->vkRelease();
    this->GrGpuBuffer::onRelease();
}

void GrVkBuffer2::onAbandon() {
    this->vkRelease();
    this->GrGpuBuffer::onAbandon();
}

void GrVkBuffer2::onMap() {
    if (!this->wasDestroyed()) {
        this->vkMap(this->size());
    }
}

void GrVkBuffer2::onUnmap() {
    if (!this->wasDestroyed()) {
        this->vkUnmap(this->size());
    }
}

bool GrVkBuffer2::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }

    if (srcSizeInBytes > this->size()) {
        return false;
    }

    if (this->isVkMappable()) {
        this->vkMap(srcSizeInBytes);
        if (!fMapPtr) {
            return false;
        }
        memcpy(fMapPtr, src, srcSizeInBytes);
        this->vkUnmap(srcSizeInBytes);
        fMapPtr = nullptr;
    } else {
        this->copyCpuDataToGpuBuffer(src, srcSizeInBytes);
    }
    return true;
}

GrVkGpu* GrVkBuffer2::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

const VkDescriptorSet* GrVkBuffer2::uniformDescriptorSet() const {
    SkASSERT(fUniformDescriptorSet);
    return fUniformDescriptorSet->descriptorSet();
}

