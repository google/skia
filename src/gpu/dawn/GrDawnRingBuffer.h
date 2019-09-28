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
#include "dawn/dawncpp.h"

class GrDawnGpu;

class GrDawnRingBuffer : public SkRefCnt {
public:
    GrDawnRingBuffer(GrDawnGpu* gpu, dawn::BufferUsageBit usage);
    ~GrDawnRingBuffer() override;

    struct Slice {
        Slice(dawn::Buffer buffer, int offset) : fBuffer(buffer), fOffset(offset) {}
        Slice() : fBuffer(nullptr), fOffset(0) {}
        Slice(const Slice& other) : fBuffer(other.fBuffer), fOffset(other.fOffset) {}
        Slice& operator=(const Slice& other) {
            fBuffer = other.fBuffer;
            fOffset = other.fOffset;
            return *this;
        }
        dawn::Buffer fBuffer;
        int fOffset;
    };
    Slice allocate(int size);

private:
    GrDawnGpu*            fGpu;
    dawn::BufferUsageBit  fUsage;
    dawn::Buffer          fBuffer;
    int                   fOffset = 0;
};

#endif
