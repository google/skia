/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnStagingBuffer_DEFINED
#define GrDawnStagingBuffer_DEFINED

#include "dawn/webgpu_cpp.h"
#include "src/gpu/GrStagingBuffer.h"

class GrDawnStagingBuffer : public GrStagingBuffer {
public:
    GrDawnStagingBuffer(GrGpu* gpu, wgpu::Buffer buffer, size_t size, void* data)
        : INHERITED(gpu, size, data), fBuffer(buffer) {}
                          ~GrDawnStagingBuffer() override {}
    void           mapAsync();
    void           onUnmap() override;
    wgpu::Buffer   buffer() const { return fBuffer; }
    bool           waiting() const { return fWaiting; }
    void           setWaiting(bool waiting) { fWaiting = waiting; }
    GrDawnGpu*     getDawnGpu() const;

private:
    wgpu::Buffer   fBuffer;
    bool           fWaiting = false;
    typedef GrStagingBuffer INHERITED;
};

#endif
