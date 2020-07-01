/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagingBufferManager_DEFINED
#define GrStagingBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrGpuBuffer.h"
#include <vector>

class GrResourceProvider;

class GrStagingBufferManager {
public:
    GrStagingBufferManager(GrResourceProvider* provider) : fResourceProvider(provider) {}

    struct Slice {
        Slice() {}
        Slice(GrGpuBuffer* buffer, size_t offset, void* offsetMapPtr)
                : fBuffer(buffer), fOffset(offset), fOffsetMapPtr(offsetMapPtr) {}
        GrGpuBuffer* fBuffer = nullptr;
        size_t fOffset = 0;
        void* fOffsetMapPtr = nullptr;
    };

    Slice allocateStagingBufferSlice(size_t size);

    using DetachBufferFunc = std::function<void(sk_sp<GrGpuBuffer>)>;

    // This call is used to move all the buffers off of the manager and to caller. This is typically
    // done at the end of the frame right before sending work to the gpu. The caller passes in a
    // detach function which will be called on each buffer in the manager. It is up to the caller
    // to take refs to the buffers in that call if they need to. After this call returns the manager
    // will have released all refs to its buffers.
    void detachBuffers(const DetachBufferFunc&);

    bool hasBuffers() { return !fBuffers.empty(); }

    void reset() {
        fBuffers.clear();
    }

private:
    static constexpr size_t kMinStagingBufferSize = 32 * 1024;

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
    GrResourceProvider* fResourceProvider;
};

#endif

