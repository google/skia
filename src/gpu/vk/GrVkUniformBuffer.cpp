/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkUniformBuffer.h"
#include "GrVkGpu.h"


GrVkUniformBuffer* GrVkUniformBuffer::Create(GrVkGpu* gpu, size_t size, bool dynamic) {
    if (0 == size) {
        return nullptr;
    }
    GrVkBuffer::Desc desc;
    desc.fDynamic = dynamic;
    desc.fType = GrVkBuffer::kUniform_Type;
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }

    GrVkUniformBuffer* buffer = new GrVkUniformBuffer(desc, bufferResource);
    if (!buffer) {
        bufferResource->unref(gpu);
    }
    return buffer;
}