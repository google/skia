/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagingBufferManager_DEFINED
#define GrStagingBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <cstddef>
#include <utility>
#include <vector>

class GrGpu;

class GrStagingBufferManager {
public:
    GrStagingBufferManager(GrGpu* gpu) : fGpu(gpu) {}

    struct Slice {
        Slice() {}
        Slice(GrGpuBuffer* buffer, size_t offset, void* offsetMapPtr)
                : fBuffer(buffer), fOffset(offset), fOffsetMapPtr(offsetMapPtr) {}
        GrGpuBuffer* fBuffer = nullptr;
        size_t fOffset = 0;
        void* fOffsetMapPtr = nullptr;
    };

    Slice allocateStagingBufferSlice(size_t size, size_t requiredAlignment = 1);

    // This call is used to move all the buffers off of the manager and to backend gpu by calling
    // the virtual GrGpu::takeOwnershipOfBuffer on each buffer. This is called during
    // submitToGpu. It is up to the backend to take refs to the buffers in their implemented
    // takeOwnershipOfBuffer implementation if they need to. After this call returns the
    // manager will have released all refs to its buffers.
    void detachBuffers();

    bool hasBuffers() { return !fBuffers.empty(); }

    void reset() {
        for (size_t i = 0; i < fBuffers.size(); ++i) {
            fBuffers[i].fBuffer->unmap();
        }
        fBuffers.clear();
    }

private:
    struct StagingBuffer {
        StagingBuffer(sk_sp<GrGpuBuffer> buffer, void* mapPtr)
                : fBuffer(std::move(buffer))
                , fMapPtr(mapPtr) {}

        sk_sp<GrGpuBuffer> fBuffer;
        void* fMapPtr;
        size_t fOffset = 0;

        size_t remaining() { return fBuffer->size() - fOffset; }
    };

    std::vector<StagingBuffer> fBuffers;
    GrGpu* fGpu;
};

#endif

