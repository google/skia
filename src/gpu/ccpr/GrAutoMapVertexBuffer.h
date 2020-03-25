/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAutoMapVertexBuffer_DEFINED
#define GrAutoMapVertexBuffer_DEFINED

#include "include/private/SkNoncopyable.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrOnFlushResourceProvider.h"

// This class automatically allocates and maps a GPU vertex buffer, and polyfills the mapping
// functionality with a mirror buffer on CPU if it is not supported.
class GrAutoMapVertexBuffer : SkNoncopyable {
public:
    ~GrAutoMapVertexBuffer() {
        if (this->isMapped()) {
            this->unmapBuffer();
        }
    }

    const GrGpuBuffer* gpuBuffer() const { return fGpuBuffer.get(); }
    bool isMapped() const { return SkToBool(fData); }
    void* data() const { SkASSERT(this->isMapped()); return fData; }

    void resetAndMapBuffer(GrOnFlushResourceProvider* onFlushRP, size_t sizeInBytes) {
        if (this->isMapped()) {
            this->unmapBuffer();
        }
        fGpuBuffer = onFlushRP->makeBuffer(GrGpuBufferType::kVertex, sizeInBytes);
        if (!fGpuBuffer) {
            fSizeInBytes = 0;
            fData = nullptr;
            return;
        }
        fSizeInBytes = sizeInBytes;
        fData = fGpuBuffer->map();
        if (!fData) {
            // Mapping failed. Allocate a mirror buffer on CPU.
            fData = sk_malloc_throw(fSizeInBytes);
        }
    }

    void unmapBuffer() {
        SkASSERT(this->isMapped());
        if (fGpuBuffer->isMapped()) {
            fGpuBuffer->unmap();
        } else {
            // fData is a mirror buffer on CPU.
            fGpuBuffer->updateData(fData, fSizeInBytes);
            sk_free(fData);
        }
        fData = nullptr;
    }

protected:
    sk_sp<GrGpuBuffer> fGpuBuffer;
    size_t fSizeInBytes = 0;
    void* fData = nullptr;
};

template<typename T> class GrTAutoMapVertexBuffer : public GrAutoMapVertexBuffer {
public:
    T& operator[](int idx) {
        SkASSERT(this->isMapped());
        SkASSERT(idx >= 0 && (size_t)idx < fSizeInBytes / sizeof(T));
        return ((T*)fData)[idx];
    }
};

#endif
