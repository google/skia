/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkIndexBuffer_DEFINED
#define GrVkIndexBuffer_DEFINED

#include "GrIndexBuffer.h"
#include "GrVkBuffer.h"
#include "vk/GrVkInterface.h"

class GrVkGpu;

class GrVkIndexBuffer : public GrIndexBuffer, public GrVkBuffer {

public:
    static GrVkIndexBuffer* Create(GrVkGpu* gpu, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkIndexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                    const GrVkBuffer::Resource* resource);

    void* onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    typedef GrIndexBuffer INHERITED;
};

#endif
