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
    bool updateData(const GrVkGpu* gpu, const void* src, size_t srcSizeInBytes) {
        return this->vkUpdateData(gpu, src, srcSizeInBytes);
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
