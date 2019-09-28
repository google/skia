/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUniformBuffer.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkUniformBuffer* GrVkUniformBuffer::Create(GrVkGpu* gpu, size_t size) {
    if (0 == size) {
        return nullptr;
    }
    const GrVkResource* resource = nullptr;
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
        resource->unref(gpu);
    }
    return buffer;
}

// We implement our own creation function for special buffer resource type
const GrVkResource* GrVkUniformBuffer::CreateResource(GrVkGpu* gpu, size_t size) {
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
        return nullptr;
    }

    const GrVkResource* resource = new GrVkUniformBuffer::Resource(buffer, alloc);
    if (!resource) {
        VK_CALL(gpu, DestroyBuffer(gpu->device(), buffer, nullptr));
        GrVkMemory::FreeBufferMemory(gpu, kUniform_Type, alloc);
        return nullptr;
    }

    return resource;
}

const GrVkBuffer::Resource* GrVkUniformBuffer::createResource(GrVkGpu* gpu,
                                                              const GrVkBuffer::Desc& descriptor) {
    const GrVkResource* vkResource;
    if (descriptor.fSizeInBytes <= GrVkUniformBuffer::kStandardSize) {
        GrVkResourceProvider& provider = gpu->resourceProvider();
        vkResource = provider.findOrCreateStandardUniformBufferResource();
    } else {
        vkResource = CreateResource(gpu, descriptor.fSizeInBytes);
    }
    return (const GrVkBuffer::Resource*) vkResource;
}

void GrVkUniformBuffer::Resource::onRecycle(GrVkGpu* gpu) const {
    if (fAlloc.fSize <= GrVkUniformBuffer::kStandardSize) {
        gpu->resourceProvider().recycleStandardUniformBufferResource(this);
    } else {
        this->unref(gpu);
    }
}
