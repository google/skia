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
    wgpu::Buffer   buffer() const { return fBuffer; }
    GrDawnGpu*     getDawnGpu() const;

private:
    void           onUnmap() override;

    wgpu::Buffer   fBuffer;
    typedef GrStagingBuffer INHERITED;
};

#endif
