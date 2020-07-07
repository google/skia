/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRingBuffer_DEFINED
#define GrRingBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"

#include "include/private/SkSpinlock.h"

/**
 * A wrapper for a GPU buffer that allocates slices in a continuous ring.
 *
 * It's assumed that suballocate and startSubmit are always called in the same thread,
 * and that finishSubmit could be called in a separate thread.
 */
class GrRingBuffer : public SkRefCnt {
public:
    GrRingBuffer(sk_sp<GrGpuBuffer> buffer, size_t size, size_t alignment)
        : fBuffer(std::move(buffer))
        , fTotalSize(size)
        , fAlignment(alignment)
        , fHead(0)
        , fTail(0)
        , fGenID(0) {
        // We increment fHead and fTail without bound and let overflow handle any wrapping.
        // Because of this, size needs to be a power of two.
        SkASSERT(SkIsPow2(size));
    }
    virtual ~GrRingBuffer() = default;

    struct Slice {
        sk_sp<GrGpuBuffer> fBuffer;
        size_t fOffset;
    };

    Slice suballocate(size_t size);

    class SubmitData {
    public:
        GrGpuBuffer* buffer() const { return fBuffer.get(); }
    private:
        friend class GrRingBuffer;
        sk_sp<GrGpuBuffer> fBuffer;
        size_t fLastHead;
        size_t fGenID;
    };
    // Backends should call startSubmit() at submit time, and finishSubmit() when the
    // command buffer/list finishes.
    SubmitData startSubmit();
    void finishSubmit(const SubmitData&);

    size_t size() const { return fTotalSize; }

private:
    virtual sk_sp<GrGpuBuffer> createBuffer(size_t size) = 0;
    size_t getAllocationOffset(size_t size);

    sk_sp<GrGpuBuffer> fBuffer;
    size_t fTotalSize;
    size_t fAlignment;
    size_t fHead SK_GUARDED_BY(fMutex);     // where we start allocating
    size_t fTail SK_GUARDED_BY(fMutex);     // where we start deallocating
    uint64_t fGenID SK_GUARDED_BY(fMutex);  // incremented when createBuffer is called
    SkSpinlock fMutex;
};

#endif
