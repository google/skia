/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRingBuffer_DEFINED
#define GrRingBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class GrGpu;
enum class GrGpuBufferType;

/**
 * A wrapper for a GPU buffer that allocates slices in a continuous ring.
 *
 * It's assumed that suballocate and startSubmit are always called in the same thread,
 * and that finishSubmit could be called in a separate thread.
 */
class GrRingBuffer {
public:
    GrRingBuffer(GrGpu* gpu, size_t size, size_t alignment, GrGpuBufferType intendedType)
        : fGpu(gpu)
        , fTotalSize(size)
        , fAlignment(alignment)
        , fType(intendedType)
        , fNewAllocation(false)
        , fHead(0)
        , fTail(0)
        , fGenID(0) {
        // We increment fHead and fTail without bound and let overflow handle any wrapping.
        // Because of this, size needs to be a power of two.
        SkASSERT(SkIsPow2(size));
    }

    struct Slice {
        GrGpuBuffer* fBuffer;
        size_t fOffset;
    };
    Slice suballocate(size_t size);

    // Backends should call startSubmit() at submit time
    void startSubmit(GrGpu*);

    size_t size() const { return fTotalSize; }

private:
    size_t getAllocationOffset(size_t size);
    struct SubmitData {
        GrRingBuffer* fOwner;
        size_t fLastHead;
        size_t fGenID;
    };
    static void FinishSubmit(void*);

    GrGpu* fGpu;
    sk_sp<GrGpuBuffer> fCurrentBuffer;
    std::vector<sk_sp<GrGpuBuffer>> fPreviousBuffers; // previous buffers we've used in this submit
    size_t fTotalSize;
    size_t fAlignment;
    GrGpuBufferType fType;
    bool fNewAllocation; // true if there's been a new allocation in this submit
    size_t fHead;        // where we start allocating
    size_t fTail;        // where we start deallocating
    uint64_t fGenID;     // incremented when createBuffer is called
};

#endif
