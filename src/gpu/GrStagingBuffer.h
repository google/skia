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
    virtual ~GrStagingBuffer() {}

    struct Slice {
        Slice() {}
        Slice(GrStagingBuffer* buffer, int offset, void* offsetMapPtr)
          : fBuffer(buffer), fOffset(offset), fOffsetMapPtr(offsetMapPtr) {}

        GrStagingBuffer*   fBuffer = nullptr; // This slice is invalid if this is nullptr
        int                fOffset = 0;
        void*              fOffsetMapPtr = nullptr; // already offset within buffer's mapPtr
    };
    size_t remaining() const { return fSize - fOffset; }

    Slice allocate(size_t size);

    // Reset the buffer to a new mapPtr and sets the offset back to 0.
    void reset(void* mapPtr) {
        fMapPtr = mapPtr;
        fOffset = 0;
    }

protected:
    GrStagingBuffer(size_t size, void* mapPtr) : fSize(size), fMapPtr(mapPtr) {}

private:
    size_t fSize;
    size_t fOffset = 0;
    void*  fMapPtr = nullptr;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrStagingBuffer);
};

#endif
