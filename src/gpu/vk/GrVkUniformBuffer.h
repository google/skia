/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformBuffer_DEFINED
#define GrVkUniformBuffer_DEFINED

#include "GrVkBuffer.h"

class GrVkGpu;

class GrVkUniformBuffer : public GrVkBuffer {

public:
    static GrVkUniformBuffer* Create(GrVkGpu* gpu, size_t size, bool dynamic);

    void* map(const GrVkGpu* gpu) {
        return this->vkMap(gpu);
    }
    void unmap(const GrVkGpu* gpu) {
        this->vkUnmap(gpu);
    }
    // The output variable createdNewBuffer must be set to true if a new VkBuffer is created in
    // order to upload the data
    bool updateData(const GrVkGpu* gpu, const void* src, size_t srcSizeInBytes,
                    bool* createdNewBuffer) {
        return this->vkUpdateData(gpu, src, srcSizeInBytes, createdNewBuffer);
    }
    void release(const GrVkGpu* gpu) {
        this->vkRelease(gpu);
    }
    void abandon() {
        this->vkAbandon();
    }

private:
    GrVkUniformBuffer(const GrVkBuffer::Desc& desc, const GrVkBuffer::Resource* resource)
        : INHERITED(desc, resource) {
    };

    typedef GrVkBuffer INHERITED;
};

#endif
