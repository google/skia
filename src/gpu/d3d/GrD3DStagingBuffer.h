/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DStagingBuffer_DEFINED
#define GrD3DStagingBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrStagingBuffer.h"

class GrD3DBuffer;

class GrD3DStagingBuffer : public GrStagingBuffer {
public:
    static std::unique_ptr<GrStagingBuffer> Make(GrGpu* gpu, size_t size,
                                                 GrStagingBuffer::Type type, void* data);
    ~GrD3DStagingBuffer() override {}

private:
    GrD3DStagingBuffer(GrGpu* gpu, size_t size, GrStagingBuffer::Type type, void* data,
                       const sk_sp<GrD3DBuffer>& buffer)
        : INHERITED(gpu, size, type, data)
        , fBuffer(buffer) {}

    void onUnmap() override {}

    sk_sp<GrD3DBuffer> fBuffer;

    typedef GrStagingBuffer INHERITED;
};

#endif
