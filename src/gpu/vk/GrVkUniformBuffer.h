/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformBuffer_DEFINED
#define GrVkUniformBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkBuffer.h"

class GrVkDescriptorSet;
class GrVkGpu;

class GrVkUniformBuffer : public GrVkBuffer {

public:
    static GrVkUniformBuffer* Create(GrVkGpu* gpu, size_t size);
    static const GrManagedResource* CreateResource(GrVkGpu* gpu, size_t size);
    static const size_t kStandardSize = 256;

    void* map(GrVkGpu* gpu) {
        return this->vkMap(gpu);
    }
    void unmap(GrVkGpu* gpu) {
        this->vkUnmap(gpu);
    }
    // The output variable createdNewBuffer must be set to true if a new VkBuffer is created in
    // order to upload the data
    bool updateData(GrVkGpu* gpu, const void* src, size_t srcSizeInBytes,
                    bool* createdNewBuffer) {
        return this->vkUpdateData(gpu, src, srcSizeInBytes, createdNewBuffer);
    }
    void release() { this->vkRelease(); }

    const VkDescriptorSet* descriptorSet() const {
        const Resource* resource = static_cast<const Resource*>(this->resource());
        return resource->descriptorSet();
    }

private:
    class Resource : public GrVkBuffer::Resource {
    public:
        Resource(GrVkGpu* gpu, VkBuffer buf, const GrVkAlloc& alloc,
                 const GrVkDescriptorSet* descSet)
            : INHERITED(gpu, buf, alloc, kUniform_Type)
            , fDescriptorSet(descSet) {}

        void freeGPUData() const override;
        void onRecycle() const override;

        const VkDescriptorSet* descriptorSet() const;

        typedef GrVkBuffer::Resource INHERITED;

    private:
        const GrVkDescriptorSet* fDescriptorSet;
    };

    const GrVkBuffer::Resource* createResource(GrVkGpu* gpu,
                                               const GrVkBuffer::Desc& descriptor) override;

    GrVkUniformBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                      const GrVkUniformBuffer::Resource* resource)
        : INHERITED(desc, resource) {}

    typedef GrVkBuffer INHERITED;
};

#endif
