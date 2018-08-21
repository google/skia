/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTBuffer_DEFINED
#define GrNXTBuffer_DEFINED

#include "GrBuffer.h"
#include "GrNXTBuffer.h"
#include "dawn/dawncpp.h"

class GrNXTGpu;
struct GrNXTStagingBuffer;

class GrNXTBuffer : public GrBuffer {
public:
    GrNXTBuffer(GrNXTGpu* gpu, size_t sizeInBytes, GrBufferType tpye, GrAccessPattern pattern);
    ~GrNXTBuffer() override;

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrNXTGpu* getNXTGpu() const;
    dawn::Buffer get() const { return fBuffer.Clone(); }

private:
    dawn::Buffer fBuffer;
    GrNXTStagingBuffer* fStagingBuffer;
    typedef GrBuffer INHERITED;
};

#endif
