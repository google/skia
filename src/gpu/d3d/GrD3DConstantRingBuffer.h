/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DConstantRingBuffer_DEFINED

#define GrD3DConstantRingBuffer_DEFINED

#include "src/gpu/GrRingBuffer.h"

class GrD3DGpu;

class GrD3DConstantRingBuffer : public GrRingBuffer {
public:
    static sk_sp<GrD3DConstantRingBuffer> Make(GrD3DGpu* gpu, size_t size, size_t alignment);

    void* mapPtr() {
        if (!buffer()->isMapped()) {
            fBufferMapPtr = buffer()->map();
        }
        return fBufferMapPtr;
    }

private:
    GrD3DConstantRingBuffer(sk_sp<GrGpuBuffer> buffer, size_t size, size_t alignment, GrD3DGpu* gpu)
        : INHERITED(std::move(buffer), size, alignment)
        , fGpu(gpu)
        , fBufferMapPtr(this->buffer()->map()) {}
    ~GrD3DConstantRingBuffer() override = default;

    sk_sp<GrGpuBuffer> createBuffer(size_t size) override;

    GrD3DGpu* fGpu;
    void* fBufferMapPtr;

    typedef GrRingBuffer INHERITED;
};

#endif
