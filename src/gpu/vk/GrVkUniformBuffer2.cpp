/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkUniformBuffer2.h"

#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

sk_sp<GrVkUniformBuffer2> GrVkUniformBuffer2::Make(GrVkGpu* gpu, size_t size, bool willSubAlloc) {
    if (0 == size) {
        return nullptr;
    }
    const GrVkUniformBuffer2::Resource* resource = GrVkUniformBuffer2::CreateResource(
            gpu, size, willSubAlloc);
    if (!resource) {
        return nullptr;
    }

    void* mapPtr = GrVkMemory::MapAlloc(gpu, resource->fAlloc);
    if (!mapPtr) {
        resource->unref();
    }

    GrVkBuffer::Desc desc;
    desc.fDynamic = true;
    desc.fType = GrVkBuffer::kUniform_Type;
    desc.fSizeInBytes = size;
    GrVkUniformBuffer2* buffer = new GrVkUniformBuffer2(
            gpu, desc, (const GrVkUniformBuffer2::Resource*)resource, mapPtr);
    if (!buffer) {
        // this will destroy anything we got from the resource provider,
        // but this avoids a conditional
        resource->unref();
    }
    return sk_sp<GrVkUniformBuffer2>(buffer);
}

// We implement our own creation function for special buffer resource type
const GrVkUniformBuffer2::Resource* GrVkUniformBuffer2::CreateResource(
        GrVkGpu* gpu, size_t size, bool willSubAlloc) {
    if (0 == size) {
        return nullptr;
    }

    VkBuffer       buffer;
    GrVkAlloc      alloc;

    // create the buffer object
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(VkBufferCreateInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.flags = 0;
    bufInfo.size = size;
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
                                              kUniform_Type,
                                              true,  // dynamic
                                              &alloc)) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        return nullptr;
    }

    const GrVkDescriptorSet* descriptorSet = gpu->resourceProvider().getUniformDescriptorSet();
    if (!descriptorSet) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        GrVkMemory::FreeBufferMemory(gpu, kUniform_Type, alloc);
        return nullptr;
    }

    VkDescriptorBufferInfo bufferInfo;
    memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    if (willSubAlloc) {
        bufferInfo.range = 128;
    } else {
        bufferInfo.range = size;
    }

    VkWriteDescriptorSet descriptorWrite;
    memset(&descriptorWrite, 0, sizeof(VkWriteDescriptorSet));
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.pNext = nullptr;
    descriptorWrite.dstSet = *descriptorSet->descriptorSet();
    descriptorWrite.dstBinding = GrVkUniformHandler::kUniformBinding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                        1,
                                                        &descriptorWrite,
                                                        0, nullptr));

    auto resource = new GrVkUniformBuffer2::Resource(gpu, buffer, alloc, descriptorSet);
    return resource;
}

const GrVkBuffer::Resource* GrVkUniformBuffer2::createResource(GrVkGpu* gpu,
                                                              const GrVkBuffer::Desc& descriptor) {
    const GrManagedResource* vkResource = CreateResource(gpu, descriptor.fSizeInBytes);
    return (const GrVkBuffer::Resource*) vkResource;
}

GrVkUniformBuffer2::GrVkUniformBuffer2(GrVkGpu* gpu,
                                       const GrVkBuffer::Desc& desc,
                                       const GrVkUniformBuffer2::Resource* resource,
                                       void* mappedPtr)
        : GrGpuBuffer(gpu, desc.fSizeInBytes, GrGpuBufferType::kUniform, kDynamic_GrAccessPattern)
        , GrVkBuffer(desc, resource, mappedPtr) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrVkGpu* GrVkUniformBuffer2::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

void GrVkUniformBuffer2::flushBufferWrites() {
    this->vkUnmap(this->getVkGpu(), true);
}

void GrVkUniformBuffer2::Resource::onRecycle() const {
    this->unref();
}

void GrVkUniformBuffer2::Resource::freeGPUData() const {
    if (fDescriptorSet) {
        fDescriptorSet->recycle();
    }
    INHERITED::freeGPUData();
}

const VkDescriptorSet* GrVkUniformBuffer2::Resource::descriptorSet() const {
    return fDescriptorSet->descriptorSet();
}

