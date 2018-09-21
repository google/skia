/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTRingBuffer_DEFINED
#define GrNXTRingBuffer_DEFINED

#include "GrBuffer.h"
#include "GrNXTBuffer.h"
#include "dawn/dawncpp.h"

class GrNXTGpu;

class GrNXTRingBuffer : public SkRefCnt {
public:
    GrNXTRingBuffer(GrNXTGpu* gpu, dawn::BufferUsageBit usage);
    ~GrNXTRingBuffer() override;

    struct Slice {
        Slice(dawn::Buffer buffer, int offset) : fBuffer(buffer.Clone()), fOffset(offset) {}
        Slice() : fBuffer(nullptr), fOffset(0) {}
        Slice(const Slice& other) : fBuffer(other.fBuffer.Clone()), fOffset(other.fOffset) {}
        Slice& operator=(const Slice& other) {
            fBuffer = other.fBuffer.Clone();
            fOffset = other.fOffset;
            return *this;
        }
        dawn::Buffer fBuffer;
        int fOffset;
    };
    Slice allocate(int size);

private:
    GrNXTGpu*             fGpu;
    dawn::BufferUsageBit  fUsage;
    dawn::Buffer          fBuffer;
    int                   fOffset = 0;
};

#endif
