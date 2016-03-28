/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkIndexBuffer_DEFINED
#define GrVkIndexBuffer_DEFINED

#include "GrBuffer.h"
#include "GrVkBuffer.h"

class GrVkGpu;

class GrVkIndexBuffer : public GrBuffer, public GrVkBuffer {

public:
    static GrVkIndexBuffer* Create(GrVkGpu* gpu, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkIndexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                    const GrVkBuffer::Resource* resource);

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    typedef GrBuffer INHERITED;
};

#endif
