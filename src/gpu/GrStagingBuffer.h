/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagingBuffer_DEFINED
#define GrStagingBuffer_DEFINED

#include "src/core/SkTInternalLList.h"

class GrGpu;

class GrStagingBuffer {
public:
    GrStagingBuffer(GrGpu* gpu, size_t size, void* mapPtr)
        : fGpu(gpu), fSize(size), fMapPtr(mapPtr) {}
    virtual ~GrStagingBuffer() {
        fGpu = nullptr;
    }
    void markAvailable(void* mapPtr);
    struct Slice {
        Slice(GrStagingBuffer* buffer, int offset, void* offsetMapPtr)
          : fBuffer(buffer), fOffset(offset), fOffsetMapPtr(offsetMapPtr) {}
        GrStagingBuffer*   fBuffer;
        int                fOffset;
        void*              fOffsetMapPtr; // already offset within buffer's mapPtr
    };
    size_t remaining() const { return fSize - fOffset; }
    GrGpu* getGpu() const { return fGpu; }
    void unmap();
    Slice allocate(size_t size);
private:
    virtual void onUnmap() = 0;

    GrGpu*                 fGpu;
    size_t                 fSize;
    size_t                 fOffset = 0;
    void*                  fMapPtr;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrStagingBuffer);
};

#endif
