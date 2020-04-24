/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkVertexBuffer_DEFINED
#define GrVkVertexBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/vk/GrVkBuffer.h"

class GrVkGpu;

class GrVkVertexBuffer : public GrGpuBuffer, public GrVkBuffer {
public:
    static sk_sp<GrVkVertexBuffer> Make(GrVkGpu* gpu, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkVertexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                     const GrVkBuffer::Resource* resource);

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    typedef GrGpuBuffer INHERITED;
};

#endif
