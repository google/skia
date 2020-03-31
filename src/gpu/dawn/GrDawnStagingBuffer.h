/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnStagingBuffer_DEFINED
#define GrDawnStagingBuffer_DEFINED

#include <map>
#include <memory>
#include <vector>

#include "dawn/webgpu_cpp.h"

class GrDawnGpu;

class GrDawnStagingBuffer {
public:
    GrDawnStagingBuffer(GrDawnGpu* gpu, wgpu::Buffer buffer, size_t size, void* data)
        : fGpu(gpu), fBuffer(buffer), fSize(size), fData(data) {}
    ~GrDawnStagingBuffer() {
        fGpu = nullptr;
    }
    void markReady(void* data);
    struct Slice {
        Slice(wgpu::Buffer buffer, int offset, void* data)
          : fBuffer(buffer), fOffset(offset), fData(data) {}
        wgpu::Buffer  fBuffer;
        int           fOffset;
        void*         fData;
    };
    bool empty() const { return fOffset == 0; }
    bool busy() const { return fBusy; }
    size_t remaining() const { return fSize - fOffset; }
    void mapAsync();
    void unmap() { fBuffer.Unmap(); }
    Slice allocate(size_t size);
private:
    GrDawnGpu*             fGpu;
    wgpu::Buffer           fBuffer;
    size_t                 fSize;
    size_t                 fOffset = 0;
    bool                   fBusy = false;
    void*                  fData;
};

#endif
