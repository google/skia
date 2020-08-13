/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnRingBuffer_DEFINED
#define GrDawnRingBuffer_DEFINED

#include "src/gpu/dawn/GrDawnBuffer.h"

class GrDawnGpu;

class GrDawnRingBuffer : public SkRefCnt {
public:
    GrDawnRingBuffer(GrDawnGpu* gpu, wgpu::BufferUsage usage);
    ~GrDawnRingBuffer() override;

    struct Slice {
        Slice(wgpu::Buffer buffer, int offset)
          : fBuffer(buffer), fOffset(offset) {}
        Slice()
          : fBuffer(nullptr), fOffset(0) {}
        Slice(const Slice& other)
          : fBuffer(other.fBuffer), fOffset(other.fOffset) {}
        Slice& operator=(const Slice& other) {
            fBuffer = other.fBuffer;
            fOffset = other.fOffset;
            return *this;
        }
        wgpu::Buffer fBuffer;
        int          fOffset;
    };
    Slice allocate(int size);

private:
    GrDawnGpu*            fGpu;
    wgpu::BufferUsage     fUsage;
    wgpu::Buffer          fBuffer;
    int                   fOffset = 0;
};

#endif
