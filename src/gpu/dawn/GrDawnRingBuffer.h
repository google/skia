/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnRingBuffer_DEFINED
#define GrDawnRingBuffer_DEFINED

#include "src/gpu/GrBuffer.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "dawn/webgpu_cpp.h"

class GrDawnGpu;

class GrDawnRingBuffer : public SkRefCnt {
public:
    GrDawnRingBuffer(GrDawnGpu* gpu, wgpu::BufferUsage usage);
    ~GrDawnRingBuffer() override;

    struct Slice {
        Slice(wgpu::Buffer buffer, int offset, void* data)
          : fBuffer(buffer), fOffset(offset), fData(data) {}
        Slice()
          : fBuffer(nullptr), fOffset(0), fData(nullptr) {}
        Slice(const Slice& other)
          : fBuffer(other.fBuffer), fOffset(other.fOffset), fData(other.fData) {}
        Slice& operator=(const Slice& other) {
            fBuffer = other.fBuffer;
            fOffset = other.fOffset;
            fData = other.fData;
            return *this;
        }
        wgpu::Buffer fBuffer;
        int          fOffset;
        void*        fData;
    };
    Slice allocate(int size);
    void flush();

private:
    GrDawnGpu*            fGpu;
    wgpu::BufferUsage     fUsage;
    wgpu::Buffer          fBuffer;
    wgpu::Buffer          fStagingBuffer;
    void*                 fData;
    int                   fOffset = 0;
};

#endif
