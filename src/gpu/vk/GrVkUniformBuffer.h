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

class GrVkGpu;

class GrVkUniformBuffer : public GrVkBuffer {

public:
    static GrVkUniformBuffer* Create(GrVkGpu* gpu, size_t size);
    static const GrVkResource* CreateResource(GrVkGpu* gpu, size_t size);
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
    void release(const GrVkGpu* gpu) { this->vkRelease(gpu); }
    void abandon() { this->vkAbandon(); }

private:
    class Resource : public GrVkBuffer::Resource {
    public:
        Resource(VkBuffer buf, const GrVkAlloc& alloc)
            : INHERITED(buf, alloc, kUniform_Type) {}

        void onRecycle(GrVkGpu* gpu) const override;

        typedef GrVkBuffer::Resource INHERITED;
    };

    const GrVkBuffer::Resource* createResource(GrVkGpu* gpu,
                                               const GrVkBuffer::Desc& descriptor) override;

    GrVkUniformBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                      const GrVkUniformBuffer::Resource* resource)
        : INHERITED(desc, resource) {}

    typedef GrVkBuffer INHERITED;
};

#endif
