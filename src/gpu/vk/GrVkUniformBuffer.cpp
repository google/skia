/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkUniformBuffer.h"

#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkUniformBuffer* GrVkUniformBuffer::Create(GrVkGpu* gpu, size_t size) {
    if (0 == size) {
        return nullptr;
    }
    const GrManagedResource* resource = nullptr;
    if (size <= GrVkUniformBuffer::kStandardSize) {
        resource = gpu->resourceProvider().findOrCreateStandardUniformBufferResource();
    } else {
        resource = CreateResource(gpu, size);
    }
    if (!resource) {
        return nullptr;
    }

    GrVkBuffer::Desc desc;
    desc.fDynamic = true;
    desc.fType = GrVkBuffer::kUniform_Type;
    desc.fSizeInBytes = size;
    GrVkUniformBuffer* buffer = new GrVkUniformBuffer(gpu, desc,
                                                      (const GrVkUniformBuffer::Resource*) resource);
    if (!buffer) {
        // this will destroy anything we got from the resource provider,
        // but this avoids a conditional
        resource->unref();
    }
    return buffer;
}

// We implement our own creation function for special buffer resource type
const GrManagedResource* GrVkUniformBuffer::CreateResource(GrVkGpu* gpu, size_t size) {
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

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                        1,
                                                        &descriptorWrite,
                                                        0, nullptr));

    const GrManagedResource* resource = new GrVkUniformBuffer::Resource(gpu, buffer, alloc,
                                                                        descriptorSet);
    return resource;
}

const GrVkBuffer::Resource* GrVkUniformBuffer::createResource(GrVkGpu* gpu,
                                                              const GrVkBuffer::Desc& descriptor) {
    const GrManagedResource* vkResource;
    if (descriptor.fSizeInBytes <= GrVkUniformBuffer::kStandardSize) {
        GrVkResourceProvider& provider = gpu->resourceProvider();
        vkResource = provider.findOrCreateStandardUniformBufferResource();
    } else {
        vkResource = CreateResource(gpu, descriptor.fSizeInBytes);
    }
    return (const GrVkBuffer::Resource*) vkResource;
}

void GrVkUniformBuffer::Resource::onRecycle() const {
    if (fAlloc.fSize <= GrVkUniformBuffer::kStandardSize) {
        fGpu->resourceProvider().recycleStandardUniformBufferResource(this);
    } else {
        this->unref();
    }
}

void GrVkUniformBuffer::Resource::freeGPUData() const {
    if (fDescriptorSet) {
        fDescriptorSet->recycle();
    }
    INHERITED::freeGPUData();
}

const VkDescriptorSet* GrVkUniformBuffer::Resource::descriptorSet() const {
    return fDescriptorSet->descriptorSet();
}

