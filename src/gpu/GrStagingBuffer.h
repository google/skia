/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagingBuffer_DEFINED
#define GrStagingBuffer_DEFINED

class GrGpu;

class GrStagingBuffer {
public:
    GrStagingBuffer(GrGpu* gpu, size_t size, void* data) : fGpu(gpu), fSize(size), fData(data) {}
    virtual ~GrStagingBuffer() {
        fGpu = nullptr;
    }
    void markReady(void* data);
    struct Slice {
        Slice(GrStagingBuffer* buffer, int offset, void* data)
          : fBuffer(buffer), fOffset(offset), fData(data) {}
        GrStagingBuffer*   fBuffer;
        int                fOffset;
        void*              fData;
    };
    bool empty() const { return fOffset == 0; }
    size_t remaining() const { return fSize - fOffset; }
    GrGpu* getGpu() const { return fGpu; }
    void unmap();
    virtual void onUnmap() = 0;
    Slice allocate(size_t size);
private:
    GrGpu*                 fGpu;
    size_t                 fSize;
    size_t                 fOffset = 0;
    void*                  fData;
};

#endif
