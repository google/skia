/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkVertexBuffer_DEFINED
#define GrVkVertexBuffer_DEFINED

#include "GrVertexBuffer.h"
#include "GrVkBuffer.h"
#include "vk/GrVkInterface.h"

class GrVkGpu;

class GrVkVertexBuffer : public GrVertexBuffer, public GrVkBuffer {
public:
    static GrVkVertexBuffer* Create(GrVkGpu* gpu, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkVertexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                     const GrVkBuffer::Resource* resource);

    void* onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    typedef GrVertexBuffer INHERITED;
};

#endif
