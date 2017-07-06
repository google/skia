/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexelBuffer_DEFINED
#define GrVkTexelBuffer_DEFINED

#include "GrBuffer.h"
#include "GrVkBuffer.h"

class GrVkGpu;

class GrVkTexelBuffer : public GrBuffer, public GrVkBuffer {
public:
    static GrVkTexelBuffer* Create(GrVkGpu* gpu, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkTexelBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                    const GrVkBuffer::Resource* resource);

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    typedef GrBuffer INHERITED;
};

#endif
